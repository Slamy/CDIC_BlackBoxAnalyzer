#include <cdfm.h>
#include <csd.h>
#include <modes.h>
#include <sysio.h>
#include <ucm.h>
#include <memory.h>
#include <stdio.h>
#include "hwreg.h"
#include "irq.h"
#include "slave.h"
#include "crc.h"
#include "framework.h"

char cdic_irq_occured = 0;
char fma_irq_occured = 0;
char fmv_irq_occured = 0;
unsigned short int_abuf = 0;
unsigned short int_xbuf = 0;
unsigned short int_dbuf = 0;
unsigned short int_audctl = 0;
unsigned short int_fma_status = 0;
unsigned short int_fmv_status = 0;
unsigned char int_dma_csr = 0;
unsigned char int_dma_csr2 = 0;
unsigned char int_dma_csr3 = 0;
unsigned int int_fma_dclk = 0;
char do_dma = 0;

unsigned short mpeg_buffer[0x500];

/* Used to store register information during a test */
/* We don't want to make any prints during the test as the baud rate is too slow */
unsigned long reg_buffer[100][40];
int bufpos = 0;
int timecnt = 0;

/* Do whatever is known to bring the CDIC into a known state */
void resetcdic()
{
	int temp;

	CDIC_ABUF = 0;
	CDIC_XBUF = 0;
	CDIC_DBUF = 0;
	CDIC_AUDCTL = 0;

	temp = CDIC_ABUF;	/* Reset IRQ flag via reading */
	temp = CDIC_XBUF;	/* Reset IRQ flag via reading */
	temp = CDIC_AUDCTL; /* Reset IRQ flag via reading */

	bufpos = 0;
	cdic_irq_occured = 0;
	int_abuf = 0;
	int_xbuf = 0;
	int_dbuf = 0;
}

void print_state()
{
	printf("State INT: %04x %04x %04x %04x  Now: %04x %04x %04x %04x\n", int_abuf, int_xbuf, int_dbuf, int_audctl, CDIC_ABUF, CDIC_XBUF, CDIC_DBUF, CDIC_AUDCTL);
	int_abuf = 0;
	int_xbuf = 0;
	int_dbuf = 0;
	int_audctl = 0;
}

#define _VA_LIST unsigned char *
#define va_start(va, paranm) (void)((va) = (_VA_LIST)__inline_va_start__())
void *__inline_va_start__(void);
#define va_end(va) (void)((va) = (_VA_LIST)0)

void print(char *format, ...)
{
#if 1
	_VA_LIST args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}

/* Overwrite CDIC driver IRQ handling */
void take_system()
{
	/* TODO I don't understand why this works for assembler code. thx to cdifan */
	store_a6();

	CDIC_IVEC = 0x2480;
	/* Only in SUPERVISOR mode, on-chip peripherals can be configured */
	/* We abuse a CDIC IRQ to set the baud rate to 19200 */
	*((unsigned long *)0x200) = SET_UART_BAUD; /* vector delivered by CDIC */
	cdic_irq_occured = 0;
	CDIC_CMD = 0x2e;	/* Command = Update */
	CDIC_DBUF = 0xc000; /* Execute command */
	while (!cdic_irq_occured)
		;

	cdic_irq_occured = 0;

	/* Switch to actual IRQ handler */
	*((unsigned long *)0x200) = CDIC_IRQ; /* vector delivered by CDIC */
	*((unsigned long *)0x1EC) = FMA_IRQ;  /* vector delivered by CDIC */
	*((unsigned long *)0x168) = FMV_IRQ;  /* vector delivered by CDIC */

#if 0
	*((unsigned long *)0xF8) = TIMER_IRQ; /* vector 62 */
	*((unsigned long *)0xF4) = VIDEO_IRQ; /* vector 61 */
	*((unsigned long *)0x68) = SLAVE_IRQ; /* vector 26 */
#endif
}

void example_crc_calculation()
{
	int i;
	unsigned short crc_accum;
	unsigned char *data[] = {0x01, 0x00, 0x02, 0x01, 0x16, 0x72, 0x00, 0x03, 0x32, 0x00, 0x53, 0xBA};

	crc_accum = 0; /* Init = 0 is assumed */
	for (i = 0; i < 12; i++)
	{
		crc_accum = CRC_CCITT_ROUND(crc_accum, (unsigned short)data[i]);
	}

	/* 0xffff is expected */
	printf("CRC Result %x\n", crc_accum);
}

void test_cmd23()
{
	printf("# test_cmd23()\n");
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	CDIC_CMD = CMD_STOP_DISC;
	CDIC_DBUF = 0xc000; /* Execute command */
	print_state();
	print_state();
	print_state();
	CDIC_DBUF = 0;
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();

	/* Output of test
	# test_cmd23()
	State INT: 7fff ffff 5801 d7fe  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff d801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0881 d7fe

	The disc has stopped
	*/
}

void test_cmd24()
{
	printf("# test_cmd24()\n");
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	CDIC_CMD = CMD_UNKNOWN_24;
	CDIC_DBUF = 0xc000; /* Execute command */
	print_state();
	print_state();
	print_state();
	CDIC_DBUF = 0;
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();
	print_state();

	/* Output of test when Audio CD is inserted
	# test_cmd24()
	State INT: 7fff ffff 5801 d7fe  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff d801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 5800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 1800 d7fe

	Output of test with Zelda - Wand of Gamelon inserted
	# test_cmd24()
	State INT: 7fff ffff 4801 d7fe  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff c801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 4801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0800 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe
	State INT: 0000 0000 0000 0000  Now: 7fff 7fff 0801 d7fe

	Note the difference: The Audio CD has bit 12 set.
	It is interesting that bit 0 of DBUF still toggles
	*/
}

/**
Prints 2 charactes to UART on every FMA Timer IRQ
It shows that one IRQ occurs every 10 ms
*/
void test_fma_timertick_irq_frequency()
{
	int i, j;

	/* I don't understand any of this, but it seems to be required */
	FMA_CMD = 1;
	FMA_IER = 0;
	print("FMA_ISR %x\n", FMA_ISR);
	FMA_R06 = 0x0900;

	FMA_IVEC = 0x807B;
	FMA_DSPA = 0;
	FMA_DSPD = 0x00F2;
	FMA_RUN = 1;
	FMA_STRM = 0;
	FMA_R04 = 0x0007;
	print("FMA_ISR %x\n", FMA_ISR);

	FMA_IER = 0x013C;
	FMA_CMD = 2;

	bufpos = 0;
	fma_irq_occured = 0;
	while (bufpos < 90)
	{
		if (fma_irq_occured)
		{
			fma_irq_occured = 0;
			printf("A\n");
		}
	}
}

/**
Analyzes the value of the DCLK register on every IRQ to
measure the frequency it counts up.
The test \ref test_fma_timertick_irq_frequency has shown
that one IRQ occurs every 10ms.
*/
void test_fma_timertick_dclk_values()
{
	int i, j;

	/* I don't understand any of this, but it seems to be required */
	FMA_CMD = 1;
	FMA_IER = 0;
	print("FMA_ISR %x\n", FMA_ISR);
	FMA_R06 = 0x0900;

	FMA_IVEC = 0x807B;
	FMA_DSPA = 0;
	FMA_DSPD = 0x00F2;
	FMA_RUN = 1;
	FMA_STRM = 0;
	FMA_R04 = 0x0007;
	print("FMA_ISR %x\n", FMA_ISR);

	FMA_IER = 0x013C;
	FMA_CMD = 2;

	bufpos = 0;
	fma_irq_occured = 0;
	while (bufpos < 90)
	{
		if (fma_irq_occured)
		{
			fma_irq_occured = 0;
			reg_buffer[bufpos][0] = int_fma_status;
			reg_buffer[bufpos][1] = int_fma_dclk;
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

	for (i = 0; i < bufpos - 1; i++)
	{
		printf("%3d %04x %04x %d\n", i, reg_buffer[i][0], reg_buffer[i][1], reg_buffer[i + 1][1] - reg_buffer[i][1]);
	}

	/*
	Example output showing that ~457 equals 10ms.
	74 0100 de56d 458
	75 0100 de737 457
	76 0100 de900 457
	77 0100 deac9 457
	78 0100 dec92 457
	79 0100 dee5b 457

	We can conclude 45700 Hz from this.
	Note how close this is to half of 90 kHz which is the the timebase of MPEG
	*/
}

/*
Plays "blue nebula" of the SkyWays Preview 20250326
https://twburn.itch.io/skyways
*/
void test_fmv_skyways()
{
	unsigned short *dbuf;
	int i, j;
	int do_dma_was_done = 1;

	CDIC_FILE = 0x0100;		/* MODE2 File filter */
	CDIC_CHAN = 0x0001;		/* We want all the channels! */
	CDIC_ACHAN = 0x0000;	/* Reset to 0, to fetch even audio sectors into normal data buffers */
	CDIC_TIME = 0x01396800; /* MSF */
	CDIC_CMD = CMD_MODE2;	/* Command = Read MODE2 */
	CDIC_DBUF = 0xc000;		/* Execute command */

	FMV_IVEC = 0x02D4;

#if 1
	FMV_VIDCMD = 0x0120;
	FMV_SYSCMD = 0x2000;
	print("FMV_ISR %x\n", FMV_ISR);
	FMV_RF4 = 0x01BF;
	FMV_RF2 = 0x1228;
	FMV_R9C = 0x0000;
	FMV_RC6 = 0x8801;
	FMV_DECOFF = 0;
	FMV_SCRPOS = 0;
	FMV_DECWIN = 0x00010000;
	FMV_IVEC = 0x02D4;

	FMV_IER = 0x2000;

	FMV_VIDCMD = 0x000C; /* WIN? */
	FMV_STRM = 0x0000;
	FMV_IMGRT = 0x0003;
	FMV_IMGSZ = 0x01800118;

	FMV_VIDCMD = 0x0020;
	FMV_RC6 = 0x882D;
	FMV_R88 = 0x0000;

	FMV_SYSCMD = 0x0100; /* RESET? */
	FMV_R92 = 0x0000;
	FMV_R88 = 0x0000;

	FMV_SYSCMD = 0x1000; /* START? */
	FMV_RA0 = 0xffff;
	FMV_TCNT = 0x0037;
	FMV_TRLD = 0x0037;
	FMV_IER = 0xf7cf;
#endif

#define WORDCNT (0x0484)

	bufpos = 0;
	fma_irq_occured = 0;
	fmv_irq_occured = 0;
	while (bufpos < 90)
	{
		if (cdic_irq_occured)
		{
			cdic_irq_occured = 0;
			dbuf = (int_dbuf & 1) ? CDIC_RAM_DBUF1 : CDIC_RAM_DBUF0;

			reg_buffer[bufpos][0] = int_dma_csr;
			reg_buffer[bufpos][1] = int_dma_csr2;
			reg_buffer[bufpos][2] = int_dma_csr3;
			reg_buffer[bufpos][3] = do_dma_was_done;
			reg_buffer[bufpos][4] = do_dma;

			if (dbuf[3] & 0x0200)
			{
				/* This is the video stream */
				for (i = 0; i < 6; i++)
				{
					FMV_XFER = dbuf[6 + i];
				}

				for (i = 0; i < WORDCNT; i++)
				{
					mpeg_buffer[i] = dbuf[12 + i];
				}

				if (do_dma_was_done > 0)
				{
					do_dma = 1;
					do_dma_was_done--;
				}
			}

			if (dbuf[3] & 0x0400)
			{
				/* This is the audio stream */
			}

			timecnt = 0;
			bufpos++;
		}

		if (fmv_irq_occured)
		{
			fmv_irq_occured = 0;

			if (int_fmv_status == 0x0900 || int_fmv_status == 0x0100)
			{
				/* Ignore timer and vsync alone */
			}
			else
			{
				reg_buffer[bufpos][0] = 0x42;
				reg_buffer[bufpos][1] = int_fmv_status;
				reg_buffer[bufpos][2] = 0;
				reg_buffer[bufpos][3] = FMV_TIMECD;
				bufpos++;
			}
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
		for (j = 0; j < 5; j++)
		{
			printf(" %04x", reg_buffer[i][j]);
		}

		printf("\n");
	}
}

int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;

	take_system();

	print("Hello CDIC!\n");

	example_crc_calculation();

	/* A freshly booted 210/05 has the audio muted.
	 * We fix that here by applying a standard attenuation.
	 */
	slave_stereo_audio_cd_attenuation();
	slave_unmute();

	/*
	test_fma_timertick_irq_frequency();
	test_fma_timertick_dclk_values();
	*/
	test_fmv_skyways();

	/* Select ONE test to execute! We don't want the tests to change each other...
	 * The reset mechanism is still not fully understood
	 */

	resetcdic();
	printf("\nTest finished. Press Ctrl-C to reset!\n");
	for (;;)
		;

	exit(0);
}
