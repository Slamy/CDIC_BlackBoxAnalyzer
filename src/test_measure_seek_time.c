#include "framework.h"
#include "hwreg.h"

static void async_seek_read(unsigned long bcd_minute) {
    CDIC_TIME = (bcd_minute << 16); /* MSF 00:MM:00 */
    CDIC_CMD = CMD_MODE1;           /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;             /* Execute command */
}

static void measure_seek_time(unsigned long minute) {
    int bcd_minute = ((minute / 10) << 4) | (minute % 10);

    unsigned long diff2;
    unsigned long diff1;
    unsigned long start = FMA_DCLK;
    /* start the seek */
    cdic_irq_occured = 0;
    async_seek_read(bcd_minute);

    while (CDIC_DBUF & 0x8000)
        ;

    diff1 = FMA_DCLK - start;

    while (!cdic_irq_occured)
        ;

    CDIC_DBUF = 0x0000; /* stop reading */
    diff2 = FMA_DCLK - start;

    reg_buffer[bufpos][0] = bcd_minute;
    reg_buffer[bufpos][1] = diff1;

    reg_buffer[bufpos][2] = *((unsigned short *)0x300000);
    reg_buffer[bufpos][3] = *((unsigned short *)0x300002);
    reg_buffer[bufpos][4] = *((unsigned short *)0x300A00);
    reg_buffer[bufpos][5] = *((unsigned short *)0x300A02);

    reg_buffer[bufpos][6] = diff2;

    bufpos++;
}

/* Keep in mind that this test will require a VMPEG DVC for time keeping */
void test_measure_seek_time() {
    unsigned long center_min = 30;
    int distance;
    int i, j;
    bufpos = 0;

    CDIC_ABUF = 0;
    CDIC_XBUF = 0;
    CDIC_DBUF = 0;
    CDIC_AUDCTL = 0;

    /* we start by seeking to the center and wait until we are there */
    measure_seek_time(center_min);
    measure_seek_time(center_min);
    measure_seek_time(center_min);

    /* now zig zag outside */
    for (distance = 0; distance < 20; distance++) {
        measure_seek_time(center_min - distance);
        measure_seek_time(center_min + distance);
    }

    for (i = 0; i < bufpos; i++) {
        printf("%3d ", i);
        for (j = 0; j < 6; j++) {
            printf(" %04x", reg_buffer[i][j]);
        }

        if (reg_buffer[i][0] != reg_buffer[i][4])
            printf(" Sector not valid");

        printf("\n");
    }

    for (i = 0; i < bufpos; i++) {
        /* factor 22.22 for us, do some fix point math here */
        unsigned long diff1_us = (reg_buffer[i][1] * 2222U) / 100U;
        unsigned long sector1_ticks = (reg_buffer[i][1] / 600) + 1;
        unsigned long diff2_us = (reg_buffer[i][6] * 2222U) / 100U;
        unsigned long sector2_ticks = (reg_buffer[i][6] / 600) + 1;
        printf("Seek to 00:%02x:00 took %d us / %d sector ticks until DBUF[15]==0 and %d us / %d sector ticks until IRQ",
               reg_buffer[i][0], diff1_us, sector1_ticks, diff2_us, sector2_ticks);

        if (reg_buffer[i][0] != reg_buffer[i][4])
            printf(" Sector not valid");

        printf("\n");
    }

    /* Measurements will vary from unit to unit. This is from my 210/05
    Seek to 00:30:00 took 119654 us /  9 sector ticks until DBUF[15]==0 and 387516 us / 30 sector ticks until IRQ
    Seek to 00:30:00 took 172804 us / 13 sector ticks until DBUF[15]==0 and 264773 us / 20 sector ticks until IRQ
    Seek to 00:30:00 took 172804 us / 13 sector ticks until DBUF[15]==0 and 264773 us / 20 sector ticks until IRQ
    Seek to 00:30:00 took  12909 us /  1 sector ticks until DBUF[15]==0 and 264773 us / 20 sector ticks until IRQ
    Seek to 00:29:00 took 239420 us / 18 sector ticks until DBUF[15]==0 and 323567 us / 25 sector ticks until IRQ
    Seek to 00:31:00 took 186136 us / 14 sector ticks until DBUF[15]==0 and 278861 us / 21 sector ticks until IRQ
    Seek to 00:28:00 took 212556 us / 16 sector ticks until DBUF[15]==0 and 309035 us / 24 sector ticks until IRQ
    Seek to 00:32:00 took 186114 us / 14 sector ticks until DBUF[15]==0 and 292526 us / 22 sector ticks until IRQ
    Seek to 00:27:00 took 186136 us / 14 sector ticks until DBUF[15]==0 and 294703 us / 23 sector ticks until IRQ
    Seek to 00:33:00 took 199446 us / 15 sector ticks until DBUF[15]==0 and 306658 us / 24 sector ticks until IRQ
    Seek to 00:26:00 took 306013 us / 23 sector ticks until DBUF[15]==0 and 412780 us / 31 sector ticks until IRQ
    Seek to 00:34:00 took 226088 us / 17 sector ticks until DBUF[15]==0 and 320812 us / 25 sector ticks until IRQ
    Seek to 00:25:00 took 292681 us / 22 sector ticks until DBUF[15]==0 and 398226 us / 30 sector ticks until IRQ
    Seek to 00:35:00 took 239420 us / 18 sector ticks until DBUF[15]==0 and 335255 us / 26 sector ticks until IRQ
    Seek to 00:24:00 took 146185 us / 11 sector ticks until DBUF[15]==0 and 251730 us / 19 sector ticks until IRQ
    Seek to 00:36:00 took 266062 us / 20 sector ticks until DBUF[15]==0 and 349387 us / 27 sector ticks until IRQ
    Seek to 00:23:00 took 266040 us / 20 sector ticks until DBUF[15]==0 and 369518 us / 28 sector ticks until IRQ
    Seek to 00:37:00 took 266040 us / 20 sector ticks until DBUF[15]==0 and 363519 us / 28 sector ticks until IRQ
    Seek to 00:22:00 took 266062 us / 20 sector ticks until DBUF[15]==0 and 355386 us / 27 sector ticks until IRQ
    Seek to 00:38:00 took 292704 us / 22 sector ticks until DBUF[15]==0 and 377695 us / 29 sector ticks until IRQ
    Seek to 00:21:00 took  12909 us /  1 sector ticks until DBUF[15]==0 and 340965 us / 26 sector ticks until IRQ
    Seek to 00:39:00 took 306013 us / 23 sector ticks until DBUF[15]==0 and 391849 us / 30 sector ticks until IRQ
    Seek to 00:20:00 took 239420 us / 18 sector ticks until DBUF[15]==0 and 326945 us / 25 sector ticks until IRQ
    Seek to 00:40:00 took 172804 us / 13 sector ticks until DBUF[15]==0 and 273350 us / 21 sector ticks until IRQ
    Seek to 00:19:00 took 212800 us / 16 sector ticks until DBUF[15]==0 and 312413 us / 24 sector ticks until IRQ
    Seek to 00:41:00 took 185892 us / 14 sector ticks until DBUF[15]==0 and 287260 us / 22 sector ticks until IRQ
    Seek to 00:18:00 took 346009 us / 26 sector ticks until DBUF[15]==0 and 430356 us / 33 sector ticks until IRQ
    Seek to 00:42:00 took 199424 us / 15 sector ticks until DBUF[15]==0 and 301747 us / 23 sector ticks until IRQ
    Seek to 00:17:00 took 306036 us / 23 sector ticks until DBUF[15]==0 and 415669 us / 32 sector ticks until IRQ
    Seek to 00:43:00 took 359297 us / 27 sector ticks until DBUF[15]==0 and 449532 us / 34 sector ticks until IRQ
    Seek to 00:16:00 took 292726 us / 22 sector ticks until DBUF[15]==0 and 401382 us / 31 sector ticks until IRQ
    Seek to 00:44:00 took 226066 us / 17 sector ticks until DBUF[15]==0 and 330189 us / 25 sector ticks until IRQ
    Seek to 00:15:00 took 292726 us / 22 sector ticks until DBUF[15]==0 and 386827 us / 30 sector ticks until IRQ

    When seeking the same sector over and over again, things are
    pretty consistent overall. Though it seems, outside things are slower?
    Might be related to the constant linear velocity of the CD

    Seek to 00:05:00 took 265106 us / 20 sector ticks
    Seek to 00:05:00 took 265106 us / 20 sector ticks
    Seek to 00:05:00 took 265129 us / 20 sector ticks
    Seek to 00:05:00 took 265129 us / 20 sector ticks
    Seek to 00:05:00 took 265106 us / 20 sector ticks
    Seek to 00:05:00 took 265106 us / 20 sector ticks
    Seek to 00:05:00 took 265129 us / 20 sector ticks
    Seek to 00:05:00 took 265129 us / 20 sector ticks
    Seek to 00:05:00 took 265106 us / 20 sector ticks
    Seek to 00:05:00 took 265106 us / 20 sector ticks
    Seek to 00:50:00 took 426112 us / 32 sector ticks
    Seek to 00:50:00 took 270195 us / 21 sector ticks
    Seek to 00:50:00 took 270217 us / 21 sector ticks
    Seek to 00:50:00 took 270195 us / 21 sector ticks
    Seek to 00:50:00 took 270350 us / 21 sector ticks
    Seek to 00:50:00 took 270195 us / 21 sector ticks
    Seek to 00:50:00 took 270217 us / 21 sector ticks
    Seek to 00:50:00 took 270217 us / 21 sector ticks
    Seek to 00:50:00 took 270195 us / 21 sector ticks
    Seek to 00:50:00 took 270217 us / 21 sector ticks
    Seek to 00:50:00 took 270195 us / 21 sector ticks
    Seek to 00:50:00 took 270217 us / 21 sector ticks
    */
}


