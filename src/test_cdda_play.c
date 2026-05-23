#include "crc.h"
#include "framework.h"
#include "hwreg.h"

/*
    Deinterleave CD subchannel R-W data.
    Generated using ChatGPT

    Input:
        raw[96]
            96 bytes of raw P-W subchannel data.

    Output:
        rw[6][12]

            rw[0] = R channel
            rw[1] = S channel
            rw[2] = T channel
            rw[3] = U channel
            rw[4] = V channel
            rw[5] = W channel

            Each channel is 96 bits = 12 bytes.
*/

unsigned short rwbuf[96];
unsigned char rw[6][12];

void deinterleave_rw_subchannels() {
    int symbol, ch;

    memset(rw, 0, 6 * 12);

    for (symbol = 0; symbol < 96; symbol++) {
        unsigned short s = rwbuf[symbol];

        for (ch = 0; ch < 6; ch++) {
            /*
                Extract bits:
                    ch=0 -> R (bit 5)
                    ch=1 -> S (bit 4)
                    ch=2 -> T (bit 3)
                    ch=3 -> U (bit 2)
                    ch=4 -> V (bit 1)
                    ch=5 -> W (bit 0)
            */

            unsigned char bit = (s >> (5 - ch)) & 1;

            if (bit) {
                rw[ch][symbol >> 3] |= (1 << (7 - (symbol & 7)));
            }
        }
    }
}

/* Start playback at 00:02:00. This test should
 * be compatible with any standard Audio CD.
 * Reading of Subchannel Q at the same time
 */
void test_cdda_play() {
    int i, j;

    unsigned short crc_accum;
    unsigned char *crc;
    unsigned short *subcode;

    printf("# test_cdda_play()\n");

    resetcdic();
    print_state();

    cdic_irq_occured = 0;

    CDIC_TIME = 0x00030000; /* MSF 00:03:00 */
    CDIC_CMD = CMD_CDDA;    /* Command = Play CDDA */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 90) {
        if (cdic_irq_occured) {
            cdic_irq_occured = 0;
            crc_accum = 0;

            /* Read subcode data */
            subcode = (int_dbuf & 1) ? 0x301324 : 0x300924;
            for (i = 0; i < 12; i++) {
                reg_buffer[bufpos][i] = subcode[i];
                crc_accum = CRC_CCITT_ROUND(crc_accum, subcode[i] & 0x00ff);
            }

            /* Store subcode RW during timecode 00:03:02 */
            if (subcode[8] == 0xff03 && subcode[9] == 0xff02) {
                subcode = (int_dbuf & 1) ? 0x300a00 : 0x300000;

                memcpy(rwbuf, subcode, 96 * 2);
            }

            reg_buffer[bufpos][12] = crc_accum;
            reg_buffer[bufpos][13] = int_dbuf;
            reg_buffer[bufpos][14] = int_audctl;
            reg_buffer[bufpos][15] = timecnt;

            timecnt = 0;

            /* Is the playback not started yet? Then play! */
            if (bufpos == 0) {
                /* Start playback. Must be performed to hear something! */
                CDIC_AUDCTL = 0x0800;
            }

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000) {
            printf("Timeout!\n");
            break;
        }
    }

    for (i = 0; i < 96; i++) {
        printf(" %02x", rwbuf[i]);
        if ((i & 0xf) == 0xf)
            printf("\n");
    }

    deinterleave_rw_subchannels();

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 12; j++) {
            printf(" %02x", rw[i][j]);
        }
        printf("\n");
    }

    for (i = 0; i < bufpos; i++) {
        printf("%3d ", i);
        for (j = 0; j < 16; j++) {
            printf(" %04x", reg_buffer[i][j]);
        }

        if (reg_buffer[i][12] == 0xffff) {
            printf("  CRC OK");
        } else {
            printf("  CRC FAIL");
        }

        printf("\n");
    }

    /*
    clang-format off
    Hello CDIC!
    CRC Result ffff
    # test_cdda_play()
    State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1801 d7fe
    ffc9 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0
    ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc9 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0
    ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0
    ffc9 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0
    ffc0 ffc0 ffc1 ffc0 ffc0 ffc0 ffc0 ffc0 ffc9 ffc0 ffc1 ffe0 ffc0 ffc0 ffc0 ffc0
    ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc0 ffc1 ffc0 ffc0 ffc0 ffc0 ffc0
    00 00 00 00 00 00 00 00 00 10 00 00
    00 00 00 00 00 00 00 00 00 00 00 00
    80 00 00 80 00 00 80 00 00 80 00 00
    00 00 00 00 00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00 00 00 00 00
    80 00 00 80 00 00 80 00 20 a0 00 20
    0  ff01 ff00 ff03 ff01 ff14 ff72 ff00 ff09 ff02 ff23 ff49 ff7c ffff d801 d7fe 000d  CRC OK
    1  ff01 ff00 ff03 ff01 ff14 ff73 ff00 ff09 ff02 ff23 ffe3 ff2d ffff d800 d7ff 01e5  CRC OK
    2  ff01 ff01 ff01 ff00 ff01 ff02 ff00 ff00 ff03 ff02 ff48 ff78 ffff 5801 d7fe d372  CRC OK
    3  ff01 ff01 ff01 ff00 ff01 ff03 ff00 ff00 ff03 ff03 fff2 ff08 ffff 5800 d7fe 02e8  CRC OK
    4  ff01 ff01 ff01 ff00 ff01 ff04 ff00 ff00 ff03 ff04 ffe5 ff3b ffff 5801 d7fe 02dd  CRC OK
    5  ff01 ff01 ff01 ff00 ff01 ff05 ff00 ff00 ff03 ff05 ff5f ff4b ffff 5800 d7fe 02fb  CRC OK
    6  ff01 ff01 ff01 ff00 ff01 ff06 ff00 ff00 ff03 ff06 ff81 fffa ffff 5801 d7fe 02fa  CRC OK
    7  ff01 ff01 ff01 ff00 ff01 ff07 ff00 ff00 ff03 ff07 ff3b ff8a ffff 5800 d7fe 02df  CRC OK
    8  ff01 ff01 ff01 ff00 ff01 ff08 ff00 ff00 ff03 ff08 ffaf ff9c ffff 5801 d7fe 02fc  CRC OK
    9  ff01 ff01 ff01 ff00 ff01 ff09 ff00 ff00 ff03 ff09 ff15 ffec ffff 5800 d7fe 02fd  CRC OK
    10  ff01 ff01 ff01 ff00 ff01 ff10 ff00 ff00 ff03 ff10 ff3a ffd2 ffff 5801 d7fe 02df  CRC OK
    And so on...
    clang-format on
    */
}

/* This test is used to check for the location where the CDDA data is
 * actually stored.
 * It turns out it is nowhere. Maybe it is directly fed to the DSP?
 */
void test_where_is_cdda() {
    int i, j;

    unsigned short crc_accum;
    unsigned char *crc;

    printf("# test_where_is_cdda()\n");

    print_state();

    cdic_irq_occured = 0;

    CDIC_TIME = 0x00020000; /* MSF 00:02:00 */
    CDIC_CMD = CMD_CDDA;    /* Command = Play CDDA */
    CDIC_DBUF = 0xc000;     /* Execute command */

    bufpos = 0;
    timecnt = 0;
    while (bufpos < 90) {
        if (cdic_irq_occured) {
            cdic_irq_occured = 0;
            crc_accum = 0;

            reg_buffer[bufpos][0] = CDIC_RAM_DBUF0[200];
            reg_buffer[bufpos][1] = CDIC_RAM_DBUF0[100];
            reg_buffer[bufpos][2] = CDIC_RAM_DBUF1[200];
            reg_buffer[bufpos][3] = CDIC_RAM_DBUF1[100];
            reg_buffer[bufpos][4] = CDIC_RAM_UNKNOWN0[200];
            reg_buffer[bufpos][5] = CDIC_RAM_UNKNOWN0[100];
            reg_buffer[bufpos][6] = CDIC_RAM_UNKNOWN1[200];
            reg_buffer[bufpos][7] = CDIC_RAM_UNKNOWN1[100];
            reg_buffer[bufpos][8] = CDIC_RAM_ADPCM0[200];
            reg_buffer[bufpos][9] = CDIC_RAM_ADPCM0[100];
            reg_buffer[bufpos][10] = CDIC_RAM_ADPCM1[200];
            reg_buffer[bufpos][11] = CDIC_RAM_ADPCM1[100];
            /* Note: Ok, it is nowhere... played back directly without
             * storage?
             */

            reg_buffer[bufpos][12] = crc_accum;
            reg_buffer[bufpos][13] = int_dbuf;
            reg_buffer[bufpos][14] = int_audctl;
            reg_buffer[bufpos][15] = timecnt;

            timecnt = 0;

            /* Is the playback not started yet? Then play! */
            if (bufpos == 0) {
                /* Start playback. Must be performed to hear something! */
                CDIC_AUDCTL = 0x0800;
            }

            bufpos++;
        }
        timecnt++;

        if (timecnt > 300000) {
            printf("Timeout!\n");
            break;
        }
    }

    for (i = 0; i < bufpos; i++) {
        printf("%3d ", i);
        for (j = 0; j < 16; j++) {
            printf(" %04x", reg_buffer[i][j]);
        }

        printf("\n");
    }
    /*
    clang-format off
    # test_where_is_cdda()
    State INT: 7fff ffff 5800 d7fe  Now: 7fff 7fff 5800 d7fe
    0  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 d7fe 101f2
    1  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02db
    2  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02dc
    3  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02e1
    4  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02e0
    5  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02da
    6  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02de
    7  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02de
    8  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02da
    9  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02e0
    10  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02df
    11  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02dc
    12  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02de
    13  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02e1
    14  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02da
    15  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5800 dffe 02dd
    16  d1f1 0000 0000 d1fc f9f2 e3e5 70b0 e0b0 3524 c020 3334 0b2f 0000 5801 dffe 02df
    And so on
    clang-format on
    */
}
