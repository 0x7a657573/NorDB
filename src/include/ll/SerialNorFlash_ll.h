/**********************************************************************
 * File : SerialNorFlash_ll.h
 * Copyright (c) 0x7a657573.
 * Created On : Sat Jul 24 2023
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#ifndef SerialNorFlash_ll_h
#define SerialNorFlash_ll_h
#include "../NorDB.h"

/*
 * Max status register reads while waiting for the chip. Sector erase takes up
 * to ~400 ms; the default covers that for polls as fast as ~0.1 us, so it never
 * times out a healthy chip. On a slow bus (~2 us per poll) a dead chip is
 * reported after ~8 s instead of hanging forever.
 */
#ifndef NORDB_FLASH_BUSY_MAX_POLL
#define NORDB_FLASH_BUSY_MAX_POLL	4000000U
#endif

typedef enum
{
	UnkownFlash = 1,
	SfdpFlash,
	TableFlash,
} FType_t;

typedef struct 
{
	uint32_t 	JEDECID;
	FType_t		ID;
	uint8_t		AddrBytes;		/*address bytes*/
	uint8_t		SectorEraseOpcode;	/*4 KB erase*/
	uint8_t		FastReadOpcode;
	uint8_t		PageProgramOpcode;
	uint32_t 	BlockCount;
	uint16_t 	PageSize;
	uint32_t 	PageCount;
	uint32_t 	SectorSize;
	uint32_t 	SectorCount;
	uint32_t 	BlockSize;
}SpiDev_t;

/*SpiBus_t must be zero-initialized before the first FlashDB_Init*/
typedef struct
{
	void *param;
	bool (*SPI_WriteRead)(void *param,uint8_t *out,uint16_t wlen,uint8_t *in,uint16_t rlen);
	
	NorDB_sem_t	 xSemaphore;
	SpiDev_t	*DevOnBus;
	uint16_t	 RefCount;	/*FlashDB handles using DevOnBus*/
	bool		 MayBeBusy;	/*chip may still be busy after a failed operation*/
}SpiBus_t;

typedef struct
{
	SpiBus_t 	 *SPI;
	uint32_t	 SectorSize;
	uint32_t	 StartOffset;
	uint32_t 	 Total_Size;
}FlashDev_t;


NorDB_HWLayer *FlashDB_Init(uint16_t StartSector,uint16_t TotalSector,SpiBus_t *spi);
void 		   Flashll_Del(NorDB_HWLayer *db);

#endif
