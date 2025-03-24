#include "ribbit_sample.h"
#include "hwreg.h"
#include "framework.h"

void collect_audiomap_registers(int marker, int timeout)
{
	/* As read by IRQ handler */
	reg_buffer[bufpos][0] = int_abuf;
	reg_buffer[bufpos][1] = CDIC_ABUF;

	reg_buffer[bufpos][2] = int_xbuf;
	reg_buffer[bufpos][3] = CDIC_XBUF;

	reg_buffer[bufpos][4] = int_dbuf;
	reg_buffer[bufpos][5] = CDIC_DBUF;

	reg_buffer[bufpos][6] = int_audctl;
	reg_buffer[bufpos][7] = CDIC_AUDCTL;

	/* Meta data */
	reg_buffer[bufpos][8] = marker;
	reg_buffer[bufpos][9] = timeout;

	int_abuf = 0;
	int_xbuf = 0;
	int_dbuf = 0;
	int_audctl = 0;

	bufpos++;
}

void observe_audiomap_registers(int maxpos, int marker)
{
	int timeout = 0;
	while (bufpos < maxpos)
	{
		if (cdic_irq_occured)
		{
			cdic_irq_occured = 0;

			collect_audiomap_registers(marker, timeout);
			timeout = 0;
		}

		timeout++;
		if (timeout > 80000)
		{
			collect_audiomap_registers(marker, timeout);
			break;
		}
	}
}

void print_reg_buffer()
{
	int i, j;
	for (i = 0; i < bufpos; i++)
	{
		print("%3d ", i);
		for (j = 0; j < 10; j++)
		{
			print(" %04x", reg_buffer[i][j]);
		}
		print("\n");
	}
}

/* Tests playback of audiomap.
 * After a certain while, the playback is stopped with 0xff coding.
 * Keep in mind that a CD must still be in the tray for this to work.
 */
void test_audiomap_play_stop()
{
	printf("# test_audiomap_play_stop()\n");

	resetcdic();

	/* [:cdic] Coding 05, 2 channels, 4 bits, 000049d4 frequency -> 106 ms between IRQs */
	/* Have this here as a backup */
	*((unsigned short *)0x30280a) = 0x0005;
	*((unsigned short *)0x30320a) = 0x0005;

	/* Copy sample data to ADPCM buffers */
	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;
	memcpy((char *)0x30280c, RibbitSoundData, 2304);
	memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

	print("Start audiomap and stop it with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;

	observe_audiomap_registers(6, 1);

	/* Gracefully stop audiomap with 0xff coding */
	*((unsigned short *)0x30280a) = 0x00ff;
	*((unsigned short *)0x30320a) = 0x00ff;

	observe_audiomap_registers(12, 2);

	print_reg_buffer();

	/* Test outtput from a 210/05
	# test_audiomap_play_stop()
	Start audiomap and stop it with 0xff coding
	0  ffff 7fff ffff 7fff 0800 0800 fffe fffe 0001 4de9
	1  ffff 7fff ffff 7fff 0800 0800 fffe fffe 0001 4fcb
	2  ffff 7fff ffff 7fff 0800 0800 fffe fffe 0001 4fbc
	3  ffff 7fff ffff 7fff 0800 0800 fffe fffe 0001 4fb7
	4  ffff 7fff ffff 7fff 0800 0800 fffe fffe 0001 4fcf
	5  ffff 7fff ffff 7fff 0800 0800 fffe fffe 0001 4fb8
	6  ffff 7fff ffff 7fff 0800 0800 f7ff f7fe 0002 4fd4
	7  0000 7fff 0000 ffff 0000 0800 0000 f7fe 0002 13881

	Notes:
	Bit 15 of ABUF is set when a single audiomap buffer has been played. (causing an IRQ?)
	Bit 0 of AUDCTL is set when the audiomap was stopped by 0xff coding
	Bit 0 of AUDCTL is reset when reading the register.
	Bit 11 is reset when the audiomap was stoppped by 0xff coding

	Bit 15 of XBUF is also set. But why? There should be no CD reading.
	*/

	print("Start audiomap again with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;
	bufpos = 0;
	observe_audiomap_registers(12, 3);
	print_reg_buffer();

	/*
	Start audiomap again with 0xff coding
	0  0000 7fff 0000 ffff 0000 0800 0000 f7ff 0003 13881

	Bit 0 of AUDCTL is set, Bit 11 is reset.
	No IRQ. No Bit 15 of ABUF.
	*/
}

/* Tests abort of audiomap by resetting AUDCTL
 */
void test_audiomap_play_abort()
{
	printf("# test_audiomap_play_abort()\n");

	resetcdic();

	/* [:cdic] Coding 05, 2 channels, 4 bits, 000049d4 frequency -> 106 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0005;
	*((unsigned short *)0x30320a) = 0x0005;

	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;
	memcpy((char *)0x30280c, RibbitSoundData, 2304);
	memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

	print("Start audiomap and abort it during playback\n");
	CDIC_AUDCTL = 0x2800;

	observe_audiomap_registers(6, 1);

	CDIC_AUDCTL = 0; /* Abort with AUDCTL reset */

	observe_audiomap_registers(12, 2);

	print_reg_buffer();

	/*
	# test_audiomap_play_abort()
	Start audiomap and abort it during playback
	0  ffff 7fff ffff 7fff 0801 0801 fffe fffe 0001 4dd1
	1  ffff 7fff ffff 7fff 0801 0801 fffe fffe 0001 4fd9
	2  ffff 7fff ffff 7fff 0801 0801 fffe fffe 0001 4fcf
	3  ffff 7fff ffff 7fff 0801 0801 fffe fffe 0001 4f96
	4  ffff 7fff ffff 7fff 0801 0801 fffe fffe 0001 4fd8
	5  ffff 7fff ffff 7fff 0801 0801 fffe fffe 0001 4fd4
	6  0000 ffff 0000 ffff 0000 0801 0000 d7fe 0002 13881

	Bit 0 of AUDCTL is not set.
	Bit 15 of ABUF is set but no IRQ is generated.
	*/
}

/* Tests another method of abort of audiomap */
void test_audiomap_play_abort_via_play_of_ff()
{
	printf("# test_audiomap_play_abort_via_play_of_ff()\n");

	resetcdic();

	/* [:cdic] Coding 05, 2 channels, 4 bits, 000049d4 frequency -> 106 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0005;
	*((unsigned short *)0x30320a) = 0x0005;

	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;
	memcpy((char *)0x30280c, RibbitSoundData, 2304);
	memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

	print("Start audiomap and stop it with 0xff coding and 0x2800 again\n");
	CDIC_AUDCTL = 0x2800;

	observe_audiomap_registers(6, 1);

	/* Gracefully stop audiomap with 0xff coding */
	*((unsigned short *)0x30280a) = 0x00ff;
	*((unsigned short *)0x30320a) = 0x00ff;

	/* But directly play! */
	CDIC_AUDCTL = 0x2800;

	observe_audiomap_registers(12, 2);

	print_reg_buffer();

	/*
	Start audiomap and stop it with 0xff coding
	0  ffff 7fff 7fff 7fff fffe 0001 4f31
	1  ffff 7fff 7fff 7fff fffe 0001 4fc8
	2  ffff 7fff 7fff 7fff fffe 0001 4fc8
	3  ffff 7fff 7fff 7fff fffe 0001 4fdd
	4  ffff 7fff 7fff 7fff fffe 0001 4fa5
	5  ffff 7fff 7fff 7fff fffe 0001 4fce
	6  ffff 7fff 7fff 7fff f7ff 0002 4fcb
	7  0000 0000 7fff 7fff f7fe 0002 13881

	Bit 15 of ABUF is set when a single audiomap buffer has been played. (causing an IRQ?)
	Bit 0 of AUDCTL is set when the audiomap was stopped by 0xff coding
	Bit 0 of AUDCTL is reset when reading the register.
	Bit 11 is reset is reset when the audiomap was stoppped by 0xff coding
	*/

	print("Start audiomap again with 0xff coding\n");
	CDIC_AUDCTL = 0x2800;
	bufpos = 0;
	observe_audiomap_registers(12, 3);
	print_reg_buffer();

	/*
	Start audiomap again with 0xff coding
	0  0000 0000 7fff 7fff f7ff 0003 13881

	Bit 0 of AUDCTL is set, Bit 11 is reset.
	No IRQ. No Bit 15 of ABUF.
	*/
}

void wait_for_abuf_int()
{
	int_abuf = 0;
	cdic_irq_occured = 0;
	for (;;)
	{
		if (cdic_irq_occured)
		{
			cdic_irq_occured = 0;
			if (int_abuf & 0x8000)
			{
				return;
			}
		}
	}
}

/* Plays the ribbit sample of frog feast once */
void test_single_ribbit()
{
	while (CDIC_AUDCTL & 0x0800)
		;

	/* Copy sample data to ADPCM buffers */
	/* [:cdic] Coding 04s, 1 channels, 4 bits, 000049d4 frequency ->  213 ms between IRQs */
	*((unsigned short *)0x30280a) = 0x0004;
	*((unsigned short *)0x30320a) = 0x0004;
	memcpy((char *)0x30280c, RibbitSoundData, 2304);
	memcpy((char *)0x30320c, RibbitSoundData + 2304, 2304);

	CDIC_AUDCTL = 0x2800;

	wait_for_abuf_int(); /* Wait for playback of 0x2800 */

	/* Gracefully stop audiomap with 0xff coding */
	*((unsigned short *)0x30280a) = 0x00ff;

	wait_for_abuf_int();  /* Wait for playback of 0x3200 */

	*((unsigned short *)0x30320a) = 0x00ff;

	if ((int_audctl & 1) == 0)
	{
		printf("Finished audiomap bit not set!\n");
		print_state();
	}
}

void test_volumes()
{
	int i;
	for (i = 0; i < 4; i++)
	{
		print("Audio CD\n");
		slave_stereo_audio_cd_attenuation();
		test_single_ribbit();

		print("Hotel Mario\n");
		slave_hotel_mario();
		test_single_ribbit();

		print("Inverted Audio CD\n");
		slave_stereo_inverted_attenuation();
		test_single_ribbit();
	}
}
