#include <cdfm.h>
#include <csd.h>
#include <modes.h>
#include <sysio.h>
#include <ucm.h>
#include <stdio.h>
#include <memory.h>
#include "video.h"
#include "graphics.h"
#include "sample.h"

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int

#define NULL 0

/* Sounds */
#define MAXSOUNDS 1

/* #define DEBUG */

int Sound;					   /* Audio device handle */
int SoundID[MAXSOUNDS * 2];	   /* Soundmap IDs. Create a set for each frog */
WORD SoundLengths[MAXSOUNDS] = /* Length of each sound */
	{
		2304 * 2};

void CreateSoundMap(Index, Filename, Length);

void InitSound()
{
	WORD Loop;
	char *DeviceName;
	char *Data = SoundData;

#ifdef DEBUG
	printf("Initialize Sound\n");
#endif

	/* Set the audio device handle to invalid */
	Sound = -1;

	/* Set sound id handle to invalid */
	for (Loop = 0; Loop < MAXSOUNDS * 2; Loop++)
	{
		SoundID[Loop] = -1;
	}

	/* Find the audio device */
	DeviceName = csd_devname(DT_AUDIO, 1);

	if (NULL == DeviceName)
	{
#ifdef DEBUG
		printf("Unable to find audio device\n");
#endif
		return;
	}

	/* Open the audio device */
	Sound = open(DeviceName, S_IREAD | S_IWRITE);

	/* Unable to open the sound device */
	if (-1 == Sound)
	{
#ifdef DEBUG
		printf("Unable to open audio device\n");
#endif
		/* Release the memory for the device name */
		free(DeviceName);

		return;
	}

	/* Release the memory for the device name, as it's no longer needed. */
	free(DeviceName);

	/* Set maximum volume in left-left and right-right channels */
	sc_atten(Sound, 0x00800080);

	/* Create soundmap for both frogs, using the same sound data */
	for (Loop = 0; Loop < MAXSOUNDS; Loop++)
	{
		CreateSoundMap(Loop, (DWORD *)Data, SoundLengths[Loop]);

		/* Advance the sound pointer to the next sound */
		Data += SoundLengths[Loop];
	}
}

void CloseSound()
{
	WORD Loop;

	/* If the Sound device isn't invalid, free the sound device, and sounds */
	if (Sound != -1)
	{
		for (Loop = 0; Loop < MAXSOUNDS; Loop++)
		{
			if (SoundID[Loop] != -1)
			{
				sm_close(Sound, SoundID[Loop]);
			}
		}

		/* Stop sound device */
		sm_off(Sound);

		/* Close the sound device */
		close(Sound);

		/* Mark the sound device as invalid */
		Sound = -1;
	}
}

#define CDFMVARS_SIZE 0x434
#define CDFMVARS_WORDS (CDFMVARS_SIZE / 2)

unsigned short cdfmvars_backup[CDFMVARS_WORDS];
unsigned short cdfmvars_last_backup[CDFMVARS_WORDS];

unsigned short events[100];
event_index = 0;

/* unsigned short *CDFMVARS = (unsigned short *)0x0efe710; */
unsigned short *CDFMVARS = (unsigned short *)0x027e710;

void StoreState()
{
	memcpy(cdfmvars_backup, CDFMVARS, sizeof(cdfmvars_backup));
}

void CompareState()
{
	int i;
	memcpy(cdfmvars_backup, CDFMVARS, sizeof(cdfmvars_backup));
	for (i = 0; i < CDFMVARS_WORDS; i++)
	{
		if (cdfmvars_last_backup[i] != cdfmvars_backup[i])
		{
#ifdef DEBUG
			printf("%d %x %x\n", i, cdfmvars_last_backup[i], cdfmvars_backup[i]);
#endif
			cdfmvars_last_backup[i] = cdfmvars_backup[i];
		}
	}
}

#if 0
void CompareState()
{
	int svirq;
	int i;
#if 0
	/* printf("A\n");*/
	svirq = irq_save();
	irq_disable(); /* mask interrupts */
	memcpy(cdfmvars_backup, CDFMVARS, sizeof(cdfmvars_backup));
	irq_restore(svirq); /* unmask interrupts */

	/* printf("B\n"); */
#endif	

	for (i = 0; i < CDFMVARS_WORDS; i++)
	{
		if (cdfmvars_last_backup[i] != cdfmvars_backup[i])
		{
#ifdef DEBUG
			printf("%d %x %x\n", i, cdfmvars_last_backup[i], cdfmvars_backup[i]);
#endif
			cdfmvars_last_backup[i] = cdfmvars_backup[i];
		}
	}
	/* memcpy(cdfmvars_last_backup, cdfmvars_backup, sizeof(cdfmvars_backup)); */
}
#endif

void CreateSoundMap(Index, Data, Length)
	WORD Index;
char *Data;
WORD Length;
{
	WORD Loop;
	char *Buffer;

	/* Create the sound map for the first frog */
#ifdef DEBUG
	printf("Creating sound map %d\n", Index + 1);
#endif

	/* Create a sound map, based on the length of the sound */
	SoundID[Index] = sm_creat(Sound, D_CMONO, Length * 18 / 2304, &Buffer);

	/* If the sound map invalid, fail out. */
	if (-1 == SoundID[Index])
	{
#ifdef DEBUG
		printf("Unable to create sound map %d\n", Index + 1);
#endif
		return;
	}

	/* Copy data to sound buffer */
	memcpy(Buffer, Data, Length);

	/* Create the sound map for the second frog */
	Index += MAXSOUNDS;

#ifdef DEBUG
	printf("Creating sound map %d\n", Index + 1);
#endif

	/* Create a sound map, based on the length of the sound */
	SoundID[Index] = sm_creat(Sound, D_CMONO, Length / 128 * 18, (char *)&Buffer);

	/* If the sound map invalid, fail out. */
	if (-1 == SoundID[Index])
	{
#ifdef DEBUG
		printf("Unable to create sound map %d\n", Index + 1);
#endif
		return;
	}

	/* Copy data to sound buffer */
	memcpy(Buffer, Data, Length);
}

#define SIG_AUDIO 0x0010
static STAT_BLK Status;
short finished = 0;
void PlaySound(SoundNumber)
	WORD SoundNumber;
{
	static AudioStatus audiostat;
	static AudioStatus last_audiostat;
	int result;

	Status.asy_sig = SIG_AUDIO;
	finished = 0;
	memset(&last_audiostat, sizeof(last_audiostat), 0);
	memset(&audiostat, sizeof(audiostat), 0);

	/* Play sound */
	sc_atten(Sound, 0x00600060);

	CompareState();

	result = sm_out(Sound, SoundID[SoundNumber], &Status);
#ifdef DEBUG
	printf("Playing sound %d -> %d\n", SoundNumber + 1, result);
#endif

	CompareState();

	result = sm_stat(Sound, &audiostat);
#ifdef DEBUG
	printf("First Stat sound %d   %d %d %d %d\n", result, audiostat.sms_sctnum, audiostat.sms_totsec, audiostat.sms_lpcnt, audiostat.sms_res);
#endif

	while (!finished)
	{
		CompareState();

		result = sm_stat(Sound, &audiostat);
#ifdef DEBUG
		if (result != 0)
			printf("Nope!\n");
#endif

		if (memcmp(&audiostat, &last_audiostat, sizeof(last_audiostat)) != 0)
		{
			last_audiostat = audiostat;
#ifdef DEBUG
			printf("Stat sound %d   %d %d %d %d\n", result, audiostat.sms_sctnum, audiostat.sms_totsec, audiostat.sms_lpcnt, audiostat.sms_res);
#endif
		}
	}
}

int intHandler(sigCode)
int sigCode;
{
	if (sigCode == SIG_BLANK)
	{
		frameDone = 1;
		frameTick++;
	}
	else if (sigCode == SIG_AUDIO)
	{
#ifdef DEBUG
		printf("X %x\n", Status.asy_stat);
#endif
		finished = 1;
	}
	else
	{
#ifdef DEBUG
		printf("%x\n", sigCode);
#endif
	}
}

#define I_BUTTON1 0x01
#define I_BUTTON2 0x02
#define I_BUTTON3 0x04
#define I_BUTTON_ANY (I_BUTTON1 | I_BUTTON2 | I_BUTTON3)
#define I_LEFT 0x10
#define I_RIGHT 0x20
#define I_UP 0x40
#define I_DOWN 0x80
#define I_SIGNAL1 0x0D00
#define I_SIGNAL2 0x0D01

int main(argc, argv)
int argc;
char *argv[];
{
	FILE *file;
	int bytes;
	int wait;
	int waitamount = 100;

	u_short input;

#ifdef DEBUG
	printf("Hello World!\n");
#endif

	intercept(intHandler);
	initVideo();
	initGraphics();
	initInput();

	InitSound();
	StoreState();

	for (;;)
	{
		for (wait = 0; wait < waitamount; wait++)
		{
			dc_ssig(videoPath, SIG_BLANK, 0);

			while (!frameDone)
			{
				CompareState();
			}; /* Wait for SIG_BLANK */
			frameDone = 0;
		}

		PlaySound(0);

		input = readInput1();

		if ((input & I_BUTTON1) && waitamount > 0)
		{
			waitamount--;
#ifdef DEBUG
			printf("%d\n", waitamount);
#endif
		}

		if ((input & I_BUTTON2) && waitamount < 100)
		{
			waitamount++;
#ifdef DEBUG
			printf("%d\n", waitamount);
#endif
		}
	}

	exit(0);
}
