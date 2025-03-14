# CDIC Registers

Due to the lack of documentation, these might not be the correct register names.
The naming is based on [this reverse engineering effort](https://github.com/cdifan/cdichips/blob/master/ims66490cdic.md)

Registers must always be written to and read from using word access.

## Command (CMD) @ 0x3C00

| Value | Description                                                   |
| ----- | ------------------------------------------------------------- |
| 0x23  | Stop disc rotation                                            |
| 0x24  | Stop reading, keep disc spinning?                             |
| 0x27  | Fetch Table of Contents                                       |
| 0x28  | Play CDDA                                                     |
| 0x29  | Read CD-I Data as Mode 1                                      |
| 0x2A  | Read CD-I Data as Mode 2 (can play ADPCM sectors without CPU) |
| 0x2B  | ? MAME thinks this stops CDDA ?                               |
| 0x2E  | Update of Mode 2 file and channel filters                     |
| 0x2C  | ? MAME calls it seek ?                                        |

The value of this register is only parsed when Bit 15 of DBUF is set

## Time (TIME) @ 0x3C02 - 0x3C04

This register is a long word

| Field   | Description  |
| ------- | ------------ |
| 31 - 24 | BCD Minute   |
| 23 - 16 | BCD Second   |
| 15 -  8 | BCD Fraction |
| 7 -  0  | Unused ?     |

## File (FILE) @ 0x3C06

| Field  | Description                                                                          |
| ------ | ------------------------------------------------------------------------------------ |
| 15 - 8 | Compared against FILE entry of MODE2 header. Sector will only be delivered on match. |
| 7 - 0  | Seems to be not used.                                                                |

## Channel mask (CHAN) @ 0x3C08 & 0x3C0A

| Field  | Description                                                                          |
| ------ | ------------------------------------------------------------------------------------ |
| 31 - 0 | Compared against CHAN entry of MODE2 header. Sector will only be delivered on match. |

The CHAN entry defines the bit position to check. The bit in this register must be set to deliver the sector.
Multiple interleaved tracks can be selected by setting more than one bit.

## Audio Channel Mask (ACHAN) @ 0x3C0C

| Field  | Description                                                                                     |
| ------ | ----------------------------------------------------------------------------------------------- |
| 15 - 0 | Compared against CHAN entry of MODE2 header. Sector will only be used as audio buffer on match. |

The CHAN entry defines the bit position to check. The bit in this register must be set to deliver the sector.
In general, only one bit should be set to avoid glitched audio.

## Data Select (DSEL) @ 0x3C80

TODO Unused?
[Only ever meantioned here](https://github.com/cdifan/cdichips/blob/master/ims66490cdic.md)

## Audio Buffer (ABUF) @ 0x3FF4

| Field  | Description                                                    |
| ------ | -------------------------------------------------------------- |
| 15     | Set by CDIC when playback of single ADPCM buffer has finished. |
|        | Reset on read. Causes IRQ when AUDCTL bit 13 is set.           |
| 14 - 0 | Always reads as all bits set ?                                 |

## Extra Buffer (XBUF) @ 0x3FF6

| Field  | Description                                         |
| ------ | --------------------------------------------------- |
| 15     | Sector buffer filled for processing. Reset on read. |
|        | Causes an IRQ but only when bit 14 of DBUF is set   |
| 14 - 0 | Always reads as all bits set ?                      |

## DMA Control (DMACTL) @ 0x3FF8

TODO

## Audio Control (AUDCTL) @ 0x3FFA

| Field   | Description                                                                          |
| ------- | ------------------------------------------------------------------------------------ |
| 15 - 14 | Always reads as all bits set ?                                                       |
| 13      | Enable Audio IRQs.                                                                   |
| 12      | Always reads as all bits set ?                                                       |
| 11      | Start playback of ADPCM buffers (from CD via Audio Channel mask or from CPU)         |
| 10 -  1 | Always reads as all bits set ?                                                       |
| 0       | Set when ADPCM playback stopped with 0xff coding.<br/>Reset on read.<br/>Causes IRQ? |

## Interrupt Vector (IVEC) @ 0x3FFC

On the CD-i, this register is set to 0x2480.
The purpose of the high byte is unknown.
The low byte is the interrupt vector. With a value of 0x80, the interrupt vector is assumed at 0x200 in memory.

Changing the high byte to something like 0x00 doesn't change anything noticable.

## Data Buffer (DBUF) @ 0x3FFE

| Field | Description                                                                         |
| ----- | ----------------------------------------------------------------------------------- |
| 15    | Set to execute command. Reset by CDIC when command is executed.                     |
| 14    | If set, data is delivered to the CPU. (Required for IRQs?)                          |
| 13    | Always 0?                                                                           |
| 12    | Sometimes set during CDDA<br/>Subcode P for Pause? (It is set between audio tracks) |
| 11    | 1 during reading? Is it always 1? TODO                                              |
| 10-8  | Always 0?                                                                           |
| 7     | Set when subcode Q has a CRC failure. Reset on read.                                |
| 6     | Always 0?                                                                           |
| 5     | Set During MODE2 reading?                                                           |
| 4     | Always 0? Maybe part of index?                                                      |
| 3-0   | Index of altered buffer? 4 and 5 for audio sectors.                                 |

