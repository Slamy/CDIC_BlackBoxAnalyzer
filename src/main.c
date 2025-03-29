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
char slave_irq_occured = 0;
unsigned short int_abuf = 0;
unsigned short int_xbuf = 0;
unsigned short int_dbuf = 0;
unsigned short int_audctl = 0;

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

	/* Switch to actual IRQ handler */
	*((unsigned long *)0x200) = CDIC_IRQ; /* vector delivered by CDIC */

	/* Switch to actual IRQ handler */
	*((unsigned long *)0x68) = SLAVE_IRQ; /* vector 26 */

#if 0
	*((unsigned long *)0xF8) = TIMER_IRQ; /* vector 62 */
	*((unsigned long *)0xF4) = VIDEO_IRQ; /* vector 61 */
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

unsigned char slave_buf[20];

int main(argc, argv)
int argc;
char *argv[];
{
	int bytes;
	int wait;
	int framecnt = 0;
	int i = 0;

	take_system();

	printf("Hello CDIC!\n");
	SLAVE_CH2 = 0x8d;

	for (;;)
	{
		i++;
		if (i == 20000)
		{
			printf("Timeout!\n");
			i = 0;
		}

		if (slave_irq_occured)
		{
			printf("Read Slave\n");

			slave_irq_occured = 0;
#if 0
			slave_buf[0] = SLAVE_CH0;
			slave_buf[1] = SLAVE_CH0;
			slave_buf[2] = SLAVE_CH0;
			slave_buf[3] = SLAVE_CH0;

			slave_buf[4] = SLAVE_CH1;
			slave_buf[5] = SLAVE_CH1;
			slave_buf[6] = SLAVE_CH1;
			slave_buf[7] = SLAVE_CH1;

			slave_buf[8] = SLAVE_CH2;
			slave_buf[9] = SLAVE_CH2;
			slave_buf[10] = SLAVE_CH2;
			slave_buf[11] = SLAVE_CH2;

			slave_buf[12] = SLAVE_CH3;
			slave_buf[13] = SLAVE_CH3;
			slave_buf[14] = SLAVE_CH3;
			slave_buf[15] = SLAVE_CH3;
#endif
			printf("%02x %02x %02x %02x\n", slave_buf[0], slave_buf[1], slave_buf[2], slave_buf[3]);
			printf("%02x %02x %02x %02x\n", slave_buf[4], slave_buf[5], slave_buf[6], slave_buf[7]);
			printf("%02x %02x %02x %02x\n", slave_buf[8], slave_buf[9], slave_buf[10], slave_buf[11]);
			printf("%02x %02x %02x %02x\n\n", slave_buf[12], slave_buf[13], slave_buf[14], slave_buf[15]);

			/*
			Press Play
			Read Slave
			ff ff ff ff
			a1 87 20 ff
			ff ff ff ff
			ff ff ff ff

			Release Play
			ff ff ff ff
			a1 87 00 ff
			ff ff ff ff
			ff ff ff ff
			*/
		}
	}

	printf("\nTest finished. Press Ctrl-C to reset!\n");
	for (;;)
		;

	exit(0);
}
