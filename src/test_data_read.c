
void test_data_read()
{
	int i, j;
	int timecnt = 0;

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
	/* Zelda - Wand of Gamelon - Map Theme*/
	CDIC_FILE = 0x0100;		/* MODE2 File filter */
	CDIC_CHAN = 0x0001;		/* MODE2 Channel filter Select which sectors to handle at all */
	CDIC_ACHAN = 0x0001;	/* Reset to 0, to fetch even audio sectors into normal data buffers */
	CDIC_TIME = 0x24362100; /* MSF 24:36:21 */
	CDIC_CMD = 0x002a;		/* Command = Read MODE2 */
	CDIC_DBUF = 0xc000;		/* Execute command */

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


			bufpos++;
		}
		timecnt++;
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
