#include "hwreg.h"

/* Reads the map theme of Zelda - Wand of Gamelon as data */
void test_mode1_read()
{
    int i, j;
    int timecnt = 0;

    printf("# test_mode1_read()\n");

    resetcdic();
    print_state();
    cdic_irq_occured = 0;

    for (i = 0; i < 16; i++)
    {
        CDIC_RAM_DBUF0[i] = 0x5555;
        CDIC_RAM_DBUF1[i] = 0x5555;
    }

    /* Does this even have an effect? */
    printf("RAM Readback %x %x \n", CDIC_RAM_DBUF0[0], CDIC_RAM_DBUF1[0]);

    /* Zelda - Wand of Gamelon - Map Theme*/
    CDIC_FILE = 0x0000;     /* MODE2 File filter is ignored here*/
    CDIC_CHAN = 0x0000;     /* Channel filter is ignored here */
    CDIC_ACHAN = 0x0000;    /* Audio channel filter is ignored here */
    CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
    CDIC_CMD = 0x0029;      /* Command = Read MODE1 */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 6)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            if (timecnt < 100)
            {
                printf("Spurious IRQ!\n");
            }

            for (i = 0; i < 16; i++)
            {
                reg_buffer[bufpos][i] = CDIC_RAM_DBUF0[i];
                reg_buffer[bufpos][i + 16] = CDIC_RAM_DBUF1[i];
            }
            reg_buffer[bufpos][32] = CDIC_DBUF;
            reg_buffer[bufpos][33] = timecnt;
            /* Reading the AUDCTL register is essential for the CDIC to produce IRQs */
            reg_buffer[bufpos][34] = CDIC_AUDCTL;

            timecnt = 0;

            bufpos++;
        }
        timecnt++;
        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            break;
        }
    }

    printf("Got %d entries\n", bufpos);

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 35; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }

        printf("\n");
    }
}
