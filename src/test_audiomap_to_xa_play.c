#include "hwreg.h"
#include "framework.h"
#include "ribbit_sample.h"

int state = 0;

#ifndef SINE_CORRUPTION_TEST

static void collect_registers()
{

    reg_buffer[bufpos][0] = CDIC_RAM_DBUF0[0];
    reg_buffer[bufpos][1] = CDIC_RAM_DBUF0[1];
    reg_buffer[bufpos][2] = CDIC_RAM_DBUF1[0];
    reg_buffer[bufpos][3] = CDIC_RAM_DBUF1[1];

    reg_buffer[bufpos][4] = int_abuf;
    reg_buffer[bufpos][5] = CDIC_ABUF;
    reg_buffer[bufpos][6] = int_xbuf;
    reg_buffer[bufpos][7] = CDIC_XBUF;
    reg_buffer[bufpos][8] = int_dbuf;
    reg_buffer[bufpos][9] = CDIC_DBUF;
    reg_buffer[bufpos][10] = int_audctl;
    reg_buffer[bufpos][11] = CDIC_AUDCTL;

    reg_buffer[bufpos][12] = timecnt;
    reg_buffer[bufpos][13] = state;
}

/* Evaluates behavior during the Help cutscene of "Zelda - Wand of Gamelon".
 * Within this cutscene the software switches frequently between audiomap and playback of audio sectors.
 * The MiSTer CD-i core (and MAME as well) has problems especially after Zelda has thrown the bomb.
 * The next spoken line should start with "Remember...". For some reason, the audio channel mask is set too late,
 * causing one missing sector of audio data.
 *
 * This test simulates the situation by also not setting the audio channel mask.
 * It turns out that this is actually ok, as the first skipped audio sector
 * contains no audible data.
 *
 * A similar scenario exists with "Zelda's Adventure" when the main character dies.
 * A short clip is played after the audiomap is disabled. The first audio sector
 * is missing but that can also be heard on a 210/05. This means,
 * this behavior is expected and emulation recreating this behavior is accurate.
 *
 * This function requires one parameter as it simulates both scenarios.
 * 0 -> Zelda - Wand of Gamelon
 * 1 -> Zelda's Adventure
 *
 * It should be noted that Wand of Gamelon has an additional problem. The audio map
 * is at low quality causing the seek time to take effect. If the seek time is too short,
 * not one but two audio sectors are skipped.
 * I'll suggest using at least 19 sectors of seeking time.
 *
 * Note: This test plays too many audio sectors. The end is therefore glitched
 * as the CDIC audioplayback is caught in a loop.
 */
void test_audiomap_to_xa_play(int target_disk)
{
    int channel;
    int i, j;
    resetcdic();

    printf("# test_audiomap_to_xa_play(%d)\n", target_disk);

    CDIC_RAM_DBUF0[0] = 0x5555;
    CDIC_RAM_DBUF0[1] = 0x5555;
    CDIC_RAM_DBUF1[0] = 0x5555;
    CDIC_RAM_DBUF1[1] = 0x5555;

    /* Let's start with some frog sounds */
    /* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
    *((unsigned short *)0x30280a) = 0x0004;
    *((unsigned short *)0x30320a) = 0x0004;
    memcpy((char *)0x30280c, RibbitSoundData, 2304);
    memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

    CDIC_AUDCTL = 0x2800;
    bufpos = 0;
    timecnt = 0;
    while (bufpos < 11)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            collect_registers();
            timecnt = 0;

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            print_state();
            break;
        }
    }

    /* Gracefully stop audiomap with 0xff coding */
    *((unsigned short *)0x30280a) = 0x00ff;
    *((unsigned short *)0x30320a) = 0x00ff;

    /* Directly start reading from disc */
    CDIC_FILE = 0x0100;  /* MODE2 File filter */
    CDIC_ACHAN = 0x0000; /* This must be 0 as the CDIC driver during MAME emulation also does it*/
    if (target_disk)
    {
        /* Zelda's Adventure */
        /* The sound that plays when you loose a life */
        channel = 0x0002;
        CDIC_TIME = 0x00455200; /* MSF 00:45:52 */
    }
    else
    {
        /* Zelda - Wand of Gamelon */
        /* Zelda - Demo cutscene - "Remember, tools can only be used when I'm standing up." */
        channel = 0x0010;
        CDIC_TIME = 0x04040800; /* MSF 04:04:08 */
    }
    CDIC_CHAN = channel;
    CDIC_CMD = CMD_MODE2; /* Command = Read MODE2 */
    CDIC_DBUF = 0xc000;   /* Execute command */
    state = 0;

    while (bufpos < 30)
    {
        if (cdic_irq_occured)
        {
            cdic_irq_occured = 0;

            collect_registers();
            timecnt = 0;

            if (state == 0 && CDIC_RAM_DBUF0[0] != 0xffc0 && int_xbuf & 0x8000)
            {
                /* Ok, I got it now. Switching to audio buffers */
                CDIC_ACHAN = channel; /* Without this, the sectors will be written to data buffers */
                CDIC_CMD = CMD_UPDATE;
                CDIC_DBUF |= 0x8000; /* Execute command */
                state = 1;
            }
            else if (state == 1 && (int_xbuf & 0x8000) && (int_audctl & 0x0800) == 0 && CDIC_ACHAN == channel)
            {
                /* Then play! */
                CDIC_AUDCTL = 0x0800;
                state = 2;
            }

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000)
        {
            printf("Timeout!\n");
            print_state();
            break;
        }
    }

    for (i = 0; i < bufpos; i++)
    {
        printf("%3d ", i);
        for (j = 0; j < 14; j++)
        {
            printf(" %04x", reg_buffer[i][j]);
        }
        printf("\n");
    }
    /*
    # test_audiomap_to_xa_play(0)
    0  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3254 0000
    1  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 329b 0000
    2  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3293 0000
    3  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3273 0000
    4  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3298 0000
    5  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 328e 0000
    6  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3293 0000
    7  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3294 0000
    8  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 328d 0000
    9  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3296 0000
    10  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0801 0801 fffe fffe 3292 0000
    11  ffc0 ffc0 ffc0 ffc0 7fff 7fff ffff 7fff c800 c800 fffe fffe 007e 0000
    12  ffc0 ffc0 ffc0 ffc0 ffff 7fff 7fff 7fff c8a0 c820 f7ff f7fe 3012 0000
    13  0404 0802 0404 0902 7fff 7fff ffff 7fff 4820 4820 f7fe f7fe 6a90 0000
    14  0404 0802 0404 2502 7fff 7fff ffff 7fff 4824 4824 f7fe f7fe 30b0 0001
    15  0404 0802 0404 4102 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30bd 0002
    16  0404 0802 0404 5702 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30af 0002
    17  0404 0802 0404 7302 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30b1 0002
    18  0404 0802 0405 1402 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30be 0002
    19  0404 0802 0405 3002 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30b0 0002
    20  0404 0802 0405 4602 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30b1 0002
    21  0404 0802 0405 6202 7fff 7fff ffff 7fff 4825 4825 dffe dffe 3094 0002
    22  0404 0802 0406 0302 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30b0 0002
    23  0404 0802 0406 1902 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30b0 0002
    24  0404 0802 0406 3502 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30bf 0002
    25  0404 0802 0406 5102 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30b3 0002
    26  0404 0802 0406 6702 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30ac 0002
    27  0404 0802 0407 0802 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30bf 0002
    28  0404 0802 0407 2402 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30b8 0002
    29  0404 0802 0407 4002 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30ac 0002

    Note: DBUF bit 7 is set during this test. It resets on read.
    It seems to be a CRC failure and has nothing to do with this scenario.

    # test_audiomap_to_xa_play(1)
    0  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3257 0000
    1  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 329d 0000
    2  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 329d 0000
    3  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 326d 0000
    4  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3299 0000
    5  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3295 0000
    6  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 328e 0000
    7  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3296 0000
    8  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3290 0000
    9  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3297 0000
    10  ffc0 ffc0 ffc0 ffc0 ffff 7fff ffff 7fff 0800 0800 fffe fffe 3293 0000
    11  ffc0 ffc0 ffc0 ffc0 7fff 7fff ffff 7fff c801 c801 fffe fffe 0062 0000
    12  ffc0 ffc0 ffc0 ffc0 ffff 7fff 7fff 7fff c8a0 c820 f7ff f7fe 3030 0000
    13  0045 5202 0045 5302 7fff 7fff ffff 7fff 4820 4820 f7fe f7fe 864e 0000
    14  0045 5202 0045 6902 7fff 7fff ffff 7fff 4824 4824 f7fe f7fe 30b0 0001
    15  0045 5202 0046 1002 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30bd 0002
    16  0045 5202 0046 2602 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30b0 0002
    17  0045 5202 0046 4202 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30b2 0002
    18  0045 5202 0046 5802 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30bf 0002
    19  0045 5202 0046 7402 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30b1 0002
    20  0045 5202 0047 1502 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30b0 0002
    21  0045 5202 0047 3102 7fff 7fff ffff 7fff 4825 4825 dffe dffe 3093 0002
    22  0045 5202 0047 4702 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30b5 0002
    23  0045 5202 0047 6302 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30ab 0002
    24  0045 5202 0048 0402 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30bf 0002
    25  0045 5202 0048 2002 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30b9 0002
    26  0045 5202 0048 3602 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30a8 0002
    27  0045 5202 0048 5202 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30be 0002
    28  0045 5202 0048 6802 7fff 7fff ffff 7fff 4824 4824 dffe dffe 30be 0002
    29  0045 5202 0049 0902 7fff 7fff ffff 7fff 4825 4825 dffe dffe 30ab 0002

    Interesting enough, the same issue occurs on the other disc. Curious.
    */
}

#endif
