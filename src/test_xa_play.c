#include "hwreg.h"

/* Plays the map theme of Zelda - Wand of Gamelon */
void test_xa_play()
{
    int i, j;
    int timecnt = 0;

    printf("# test_xa_play()\n");

    resetcdic();

    cdic_irq_occured = 0;
    print_state();

    CDIC_AUDCTL = 0;
    CDIC_ACHAN = 0;
    CDIC_CMD = 0x002e;
    CDIC_DBUF = 0xC000;

    while (!cdic_irq_occured)
        ;

    print_state();

    CDIC_DBUF = 0;
    cdic_irq_occured = 0;

    CDIC_AUDCTL = 0;
    CDIC_ACHAN = 0;
    CDIC_CMD = 0x002e;
    CDIC_DBUF = 0xC000;

    while (!cdic_irq_occured)
        ;

    print_state();
    cdic_irq_occured = 0;

    CDIC_DBUF = 0;

    /* Some random data into the buffers so we know what has changed */
    *((unsigned short *)0x300000) = 0x5555;
    *((unsigned short *)0x300002) = 0x5555;
    *((unsigned short *)0x300A00) = 0x5555;
    *((unsigned short *)0x300A02) = 0x5555;
    *((unsigned short *)0x301400) = 0x5555;
    *((unsigned short *)0x301402) = 0x5555;
    *((unsigned short *)0x301E00) = 0x5555;
    *((unsigned short *)0x301E02) = 0x5555;
    *((unsigned short *)0x302800) = 0x5555;
    *((unsigned short *)0x302802) = 0x5555;
    *((unsigned short *)0x303200) = 0x5555;
    *((unsigned short *)0x303202) = 0x5555;

    /* Zelda - Wand of Gamelon - Map Theme*/
    CDIC_FILE = 0x0100;     /* MODE2 File filter */
    CDIC_CHAN = 0x0001;     /* MODE2 Channel filter Select which sectors to handle at all */
    CDIC_ACHAN = 0x0001;    /* Without this, the sectors will be written to data buffers */
    CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
    CDIC_CMD = 0x002a;      /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 90)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            reg_buffer[bufpos][0] = *((unsigned short *)0x300000);
            reg_buffer[bufpos][1] = *((unsigned short *)0x300002);
            reg_buffer[bufpos][2] = *((unsigned short *)0x300A00);
            reg_buffer[bufpos][3] = *((unsigned short *)0x300A02);
            reg_buffer[bufpos][4] = *((unsigned short *)0x301400);
            reg_buffer[bufpos][5] = *((unsigned short *)0x301402);
            reg_buffer[bufpos][6] = *((unsigned short *)0x301E00);
            reg_buffer[bufpos][7] = *((unsigned short *)0x301E02);
            reg_buffer[bufpos][8] = *((unsigned short *)0x302800);
            reg_buffer[bufpos][9] = *((unsigned short *)0x302802);
            reg_buffer[bufpos][10] = *((unsigned short *)0x303200);
            reg_buffer[bufpos][11] = *((unsigned short *)0x303202);

            reg_buffer[bufpos][12] = int_abuf;
            reg_buffer[bufpos][13] = int_xbuf;
            reg_buffer[bufpos][14] = CDIC_DBUF;
            /* CDIC driver reads channel and audio channel. But this is not essential*/
            reg_buffer[bufpos][15] = CDIC_AUDCTL;
            reg_buffer[bufpos][16] = timecnt;
            timecnt = 0;

            /* Is the playback not started yet? Do we have an audio sector in the ADPCM buffer 0? Then play! */
            if ((CDIC_AUDCTL & 0x0800) == 0 && (CDIC_DBUF & 0x000f) == 0x0004)
            {
                /* Start playback. Must be performed to hear something */
                CDIC_AUDCTL = 0x0800;
            }

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            break;
        }
    }

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 17; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }
#if 0
		if (i & 2)
		{
			slave_stereo_audio_cd_attenuation();
		}
		else
		{
			slave_stereo_inverted_attenuation();
		}
#endif

        printf("\n");
    }
}