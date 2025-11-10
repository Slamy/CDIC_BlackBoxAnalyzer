#include "hwreg.h"
#include "framework.h"

static void async_seek_read(unsigned long bcd_minute)
{
    CDIC_TIME = (bcd_minute << 16); /* MSF 00:MM:00 */
    CDIC_CMD = CMD_MODE1;           /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;             /* Execute command */
}

static void measure_seek_time(unsigned long minute)
{
    int bcd_minute = ((minute / 10) << 4) | (minute % 10);

    unsigned long diff;
    unsigned long start = FMA_DCLK;
    /* start the seek */
    cdic_irq_occured = 0;
    async_seek_read(bcd_minute);

    while (!cdic_irq_occured)
        ;

    CDIC_DBUF = 0x0000; /* stop reading */
    diff = FMA_DCLK - start;
    reg_buffer[bufpos][0] = bcd_minute;
    reg_buffer[bufpos][1] = diff;

    reg_buffer[bufpos][2] = *((unsigned short *)0x300000);
    reg_buffer[bufpos][3] = *((unsigned short *)0x300002);
    reg_buffer[bufpos][4] = *((unsigned short *)0x300A00);
    reg_buffer[bufpos][5] = *((unsigned short *)0x300A02);

    bufpos++;
}

/* Keep in mind that this test will require a VMPEG DVC for time keeping */
void test_measure_seek_time()
{
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
    for (distance = 0; distance < 20; distance++)
    {
        measure_seek_time(center_min - distance);
        measure_seek_time(center_min + distance);
    }

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 6; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }

        if (reg_buffer[i][0] != reg_buffer[i][4])
            printf(" Sector not valid");

        printf("\n");
    }

    for (i = 0; i < bufpos; i++)
    {
        /* factor 22.22 for us, do some fix point math here */
        unsigned long diff_us = (reg_buffer[i][1] * 2222U) / 100U;
        unsigned long sector_ticks = (reg_buffer[i][1] / 600) + 1;
        printf("Seek to 00:%02x:00 took %d us / %d sector ticks", reg_buffer[i][0], diff_us, sector_ticks);

        if (reg_buffer[i][0] != reg_buffer[i][4])
            printf(" Sector not valid");

        printf("\n");
    }

    /* Measurements will vary from unit to unit. This is from my 210/05
    Seek to 00:30:00 took 777722 us / 59 sector ticks
    Seek to 00:30:00 took 267795 us / 21 sector ticks
    Seek to 00:30:00 took 267773 us / 21 sector ticks
    Seek to 00:30:00 took 267773 us / 21 sector ticks
    Seek to 00:30:00 took 267773 us / 21 sector ticks
    Seek to 00:29:00 took 339743 us / 26 sector ticks
    Seek to 00:31:00 took 257729 us / 20 sector ticks
    Seek to 00:28:00 took 349520 us / 27 sector ticks
    Seek to 00:32:00 took 247508 us / 19 sector ticks
    Seek to 00:27:00 took 359208 us / 27 sector ticks
    Seek to 00:33:00 took 371718 us / 28 sector ticks
    Seek to 00:26:00 took 369385 us / 28 sector ticks
    Seek to 00:34:00 took 361563 us / 28 sector ticks
    Seek to 00:25:00 took 378739 us / 29 sector ticks
    Seek to 00:35:00 took 351320 us / 27 sector ticks
    Seek to 00:24:00 took 388850 us / 30 sector ticks
    Seek to 00:36:00 took 341521 us / 26 sector ticks
    Seek to 00:23:00 took 398515 us / 30 sector ticks
    Seek to 00:37:00 took 331033 us / 25 sector ticks
    Seek to 00:22:00 took 408181 us / 31 sector ticks
    Seek to 00:38:00 took 321256 us / 25 sector ticks
    Seek to 00:21:00 took 418513 us / 32 sector ticks
    Seek to 00:39:00 took 310902 us / 24 sector ticks
    Seek to 00:20:00 took 428179 us / 33 sector ticks
    Seek to 00:40:00 took 435378 us / 33 sector ticks
    Seek to 00:19:00 took 437867 us / 33 sector ticks
    Seek to 00:41:00 took 290748 us / 22 sector ticks
    Seek to 00:18:00 took 581097 us / 44 sector ticks
    Seek to 00:42:00 took 415780 us / 32 sector ticks
    Seek to 00:17:00 took 457176 us / 35 sector ticks
    Seek to 00:43:00 took 405448 us / 31 sector ticks
    Seek to 00:16:00 took 466953 us / 36 sector ticks
    Seek to 00:44:00 took 395249 us / 30 sector ticks
    Seek to 00:15:00 took 343565 us / 26 sector ticks
    Seek to 00:45:00 took 385383 us / 29 sector ticks
    Seek to 00:14:00 took 352742 us / 27 sector ticks
    Seek to 00:46:00 took 375495 us / 29 sector ticks
    Seek to 00:13:00 took 362630 us / 28 sector ticks
    Seek to 00:47:00 took 365319 us / 28 sector ticks
    Seek to 00:12:00 took 372540 us / 28 sector ticks
    Seek to 00:48:00 took 355520 us / 27 sector ticks
    Seek to 00:11:00 took 514037 us / 39 sector ticks
    Seek to 00:49:00 took 479996 us / 37 sector ticks

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