#include "slave.h"
#include "hwreg.h"

void slave_mute()
{
	SLAVE_CH2 = 0x82;
}

void slave_unmute()
{
	SLAVE_CH2 = 0x83;
}

void slave_stereo_inverted_attenuation()
{
	/* From Zelda - Wand of Gamelon */
	SLAVE_CH2 = 0xca;
	SLAVE_CH2 = 0x7f;
	SLAVE_CH2 = 0x00;
	SLAVE_CH2 = 0x7f;
	SLAVE_CH2 = 0x00;
}

void slave_stereo_audio_cd_attenuation()
{
	/* As used by the Audio CD player */
	SLAVE_CH2 = 0xc5;
	SLAVE_CH2 = 0x00;
	SLAVE_CH2 = 0x00;
	SLAVE_CH2 = 0x00;
	SLAVE_CH2 = 0x00;
}

void slave_hotel_mario()
{
	/* Hotel Mario - Everything on max.
	 * Left to right, Right to left,
	 * Everything in the center!
	 */
	SLAVE_CH2 = 0xc0;
	SLAVE_CH2 = 0x00;
	SLAVE_CH2 = 0x00;
	SLAVE_CH2 = 0x00;
	SLAVE_CH2 = 0x00;
}

