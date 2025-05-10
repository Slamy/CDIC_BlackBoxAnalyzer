#ifndef _HWREG_H
#define _HWREG_H

/* --- FMA ---  */
#define FMA_CMD (*((unsigned short *)0x0E03000))
#define FMA_R04 (*((unsigned short *)0x0E03004))
#define FMA_R06 (*((unsigned short *)0x0E03006))
#define FMA_STRM (*((unsigned short *)0x0E03008))
#define FMA_DCLK (*((unsigned long *)0x0E03010))
#define FMA_RUN (*((unsigned short *)0x0E03018))
#define FMA_ISR (*((unsigned short *)0x0E0301A))
#define FMA_IER (*((unsigned short *)0x0E0301C))
#define FMA_DSPA (*((unsigned short *)0x0E03022))
#define FMA_DSPD (*((unsigned short *)0x0E03024))
#define FMA_IVEC (*((unsigned short *)0x0E0300C))

/* --- IMS66490 CD-Interface Controller (CDIC) ---  */

/* Known buffers in RAM, filled by the CDIC */

#define CDIC_RAM_DBUF0 (((unsigned short *)0x300000))
#define CDIC_RAM_DBUF1 (((unsigned short *)0x300A00))
#define CDIC_RAM_UNKNOWN0 (((unsigned short *)0x301400))
#define CDIC_RAM_UNKNOWN1 (((unsigned short *)0x301E00))
#define CDIC_RAM_ADPCM0 (((unsigned short *)0x302800))
#define CDIC_RAM_ADPCM1 (((unsigned short *)0x303200))

/* Assumed to be data in RAM, read by microcode */

#define CDIC_CMD (*((unsigned short *)0x303C00))
#define CDIC_TIME (*((unsigned long *)0x303C02))
#define CDIC_FILE (*((unsigned short *)0x303C06))
#define CDIC_CHAN (*((unsigned long *)0x303C08))
#define CDIC_ACHAN (*((unsigned short *)0x303C0C))

/* Physically stops the rotation. Reset Mode 1 in MAME */
#define CMD_STOP_DISC 0x0023
/* MAME calls it Reset Mode 2. Called once at system reset */
#define CMD_UNKNOWN_24 0x0024
#define CMD_TOC 0x0027   /* Fetches TOC */
#define CMD_CDDA 0x0028  /* Plays CDDA */
#define CMD_MODE1 0x0029 /* Reads anything */
#define CMD_MODE2 0x002a /* Only MODE2 sectors */
/* MAME says Stop CDDA. But the command seems to have no effect */
#define CMD_UNKNOWN_2b 0x002b
#define CMD_UPDATE 0x002e /* Updating MODE2 filter parameters */
/* MAME says Seek and does MODE1 reading */
#define CMD_UNKNOWN_2C 0x002c /* What purpose? */

/* What is DSEL at 3C80 ? Is it ever used */

/* Assumed to be real registers */

#define CDIC_ABUF (*((unsigned short *)0x303FF4))
#define CDIC_XBUF (*((unsigned short *)0x303FF6))
#define CDIC_DMACTL (*((unsigned short *)0x303FF8))
#define CDIC_AUDCTL (*((unsigned short *)0x303FFA)) /* Called Z buffer in MAME */
#define CDIC_IVEC (*((unsigned short *)0x303FFC))
#define CDIC_DBUF (*((unsigned short *)0x303FFE))

/* --- Slave Controller --- */

#define SLAVE_CH0 (*((unsigned char *)0x310001))
#define SLAVE_CH1 (*((unsigned char *)0x310003))
#define SLAVE_CH2 (*((unsigned char *)0x310005))

/* --- SCC68070 DMA --- */

#define DMA_CH0_STATUS (*((unsigned byte *)0x80004000))
#define DMA_CH0_ERROR (*((unsigned byte *)0x80004001))
#define DMA_CH0_DEV_CTRL (*((unsigned byte *)0x80004004))
#define DMA_CH0_OP_CTRL (*((unsigned byte *)0x80004005))
#define DMA_CH0_SEQ_CTRL (*((unsigned byte *)0x80004006))
#define DMA_CH0_CHAN_CTRL (*((unsigned byte *)0x80004007))
#define DMA_CH0_MEM_TRANS_CNT (*((unsigned short *)0x8000400a))
#define DMA_CH0_MEM_ADR (*((unsigned long *)0x8000400c))
#define DMA_CH0_DEV_ADR (*((unsigned long *)0x80004014))

/* --- SCC68070 UART --- */

#define UART_CSR (*((unsigned char *)0x80002015))

#endif
