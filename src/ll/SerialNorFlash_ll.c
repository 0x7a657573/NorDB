/**********************************************************************
 * File : SerialNorFlash_ll.c
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

#include "ll/SerialNorFlash_ll.h"

#define debug_info 	//printf

bool Searching_ForDevice(SpiBus_t *spi)
{
	uint8_t wr_buff[32]={0x9f};
    uint8_t rd_buff[32]={0xff};
	spi->SPI_WriteRead(spi->param,wr_buff,1,rd_buff,3);
	spi->DevOnBus->JEDECID = (rd_buff[0] << 16) | (rd_buff[1] << 8) | rd_buff[2]; //JEDEC ID
	
	switch (spi->DevOnBus->JEDECID & 0x000000FF)
	{
	case 0x20: // 	w25q512
		spi->DevOnBus->ID = W25Q512;
		spi->DevOnBus->BlockCount = 1024;
		debug_info("w25qxx Chip: w25q512\r\n");
		break;
	case 0x19: // 	w25q256
		spi->DevOnBus->ID = W25Q256;
		spi->DevOnBus->BlockCount = 512;
		debug_info("w25qxx Chip: w25q256\r\n");
		break;
	case 0x18: // 	w25q128
		spi->DevOnBus->ID = W25Q128;
		spi->DevOnBus->BlockCount = 256;
		debug_info("w25qxx Chip: w25q128\r\n");
		break;
	case 0x17: //	w25q64
		spi->DevOnBus->ID = W25Q64;
		spi->DevOnBus->BlockCount = 128;
		debug_info("w25qxx Chip: w25q64\r\n");
		break;
	case 0x16: //	w25q32
		spi->DevOnBus->ID = W25Q32;
		spi->DevOnBus->BlockCount = 64;
		debug_info("w25qxx Chip: w25q32\r\n");
		break;
	case 0x15: //	w25q16
		spi->DevOnBus->ID = W25Q16;
		spi->DevOnBus->BlockCount = 32;
		debug_info("w25qxx Chip: w25q16\r\n");
		break;
	case 0x14: //	w25q80
		spi->DevOnBus->ID = W25Q80;
		spi->DevOnBus->BlockCount = 16;
		debug_info("w25qxx Chip: w25q80\r\n");
		break;
	case 0x13: //	w25q40
		spi->DevOnBus->ID = W25Q40;
		spi->DevOnBus->BlockCount = 8;
		debug_info("w25qxx Chip: w25q40\r\n");
		break;
	case 0x12: //	w25q20
		spi->DevOnBus->ID = W25Q20;
		spi->DevOnBus->BlockCount = 4;
		debug_info("w25qxx Chip: w25q20\r\n");
		break;
	case 0x11: //	w25q10
		spi->DevOnBus->ID = W25Q10;
		spi->DevOnBus->BlockCount = 2;
		debug_info("w25qxx Chip: w25q10\r\n");
		break;
	default:
		spi->DevOnBus->ID = UnkownFlash;
		debug_info("w25qxx Unknown ID\r\n");
		return false;
	}

	spi->DevOnBus->PageSize = 256;
	spi->DevOnBus->SectorSize = 0x1000;
	spi->DevOnBus->SectorCount = spi->DevOnBus->BlockCount * 16;
	spi->DevOnBus->PageCount = (spi->DevOnBus->SectorCount * spi->DevOnBus->SectorSize) / spi->DevOnBus->PageSize;
	spi->DevOnBus->BlockSize = spi->DevOnBus->SectorSize * 16;
	
	debug_info("Flash Page Size: %d Bytes\r\n", spi->DevOnBus->PageSize);
	debug_info("Flash Page Count: %d\r\n", spi->DevOnBus->PageCount);
	debug_info("Flash Sector Size: %d Bytes\r\n", spi->DevOnBus->SectorSize);
	debug_info("Flash Sector Count: %d\r\n", spi->DevOnBus->SectorCount);
	debug_info("Flash Block Size: %d Bytes\r\n", spi->DevOnBus->BlockSize);
	debug_info("Flash Block Count: %d\r\n", spi->DevOnBus->BlockCount);
	debug_info("Flash Init Done\r\n");
	return true;
}

void FLASH_WriteEnable(void *Param)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	//debug_info("w25qxx Write Enable\r\n");
	uint8_t  cmd[8] = {0};
	cmd[0] = 0x06;
	spi->SPI_WriteRead(spi->param,cmd,1,NULL,0);
}

void FLASH_WaitForWriteEnd(void *Param)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	uint8_t  StatusRegister1 = 0;
	uint8_t  cmd[8] = {0};
	cmd[0] = 0x05;

	//debug_info("w25qxx wait for flash ready\r\n");
	do
	{
		spi->SPI_WriteRead(spi->param,cmd,1,&StatusRegister1,1);
	} while ((StatusRegister1 & 0x01) == 0x01);
}

void FLASH_EraseSector(void *Param,uint32_t SectorAddr)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	NorDB_sem_Lock(&spi->xSemaphore);
	/*add Sector Offset*/
	SectorAddr += FL->StartOffset;
	debug_info("w25qxx EraseSector %d Begin...\r\n", SectorAddr);
	
	FLASH_WaitForWriteEnd(Param);
	FLASH_WriteEnable(Param);

	uint8_t  cmd[8] = {0};
	uint32_t i = 0;
	
	if (spi->DevOnBus->ID >= W25Q256)
	{
		cmd[i++] = (0x21);
		cmd[i++] = ((SectorAddr & 0xFF000000) >> 24);
	}
	else
	{
		cmd[i++] = (0x20);
	}
	cmd[i++] = ((SectorAddr & 0xFF0000) >> 16);
	cmd[i++] = ((SectorAddr & 0xFF00) >> 8);
	cmd[i++] = (SectorAddr & 0xFF);
	spi->SPI_WriteRead(spi->param,cmd,i,NULL,0);
	
	FLASH_WaitForWriteEnd(Param);

	debug_info("w25qxx EraseSector done.\r\n");
	NorDB_sem_Unlock(&spi->xSemaphore);
}

void FLASH_Erase(void*Param)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;

	for(uint32_t i=0;i<FL->Total_Size;i+=spi->DevOnBus->SectorSize)
	{
		FLASH_EraseSector(Param,i);
	}
}

void FLASH_ReadBuffer(void*Param, uint32_t ReadAddr, uint8_t *pBuffer,  uint16_t NumByteToRead)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	NorDB_sem_Lock(&spi->xSemaphore);
	/*add Sector Offset*/
	ReadAddr += FL->StartOffset;

	debug_info("w25qxx ReadBytes at Address:%d, %d Bytes\r\n", ReadAddr, NumByteToRead);

	uint32_t Index = 0;
	uint8_t  cmd[8] = {0};
	if (spi->DevOnBus->ID >= W25Q256)
	{
		cmd[Index++] = 0x0C;
		cmd[Index++] = ((ReadAddr & 0xFF000000) >> 24);
	}
	else
	{
		cmd[Index++] = (0x0B);
	}
	cmd[Index++] = ((ReadAddr & 0xFF0000) >> 16);
	cmd[Index++] = ((ReadAddr & 0xFF00) >> 8);
	cmd[Index++] = (ReadAddr & 0xFF);
	cmd[Index++] = (0);
	spi->SPI_WriteRead(spi->param,cmd,Index,pBuffer,NumByteToRead);
	NorDB_sem_Unlock(&spi->xSemaphore);
}

static inline void FLASH_WritePage(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;

	FLASH_WriteEnable(Param);
	
	uint8_t cmd[512] = {0};
	uint32_t i = 0;

	if (spi->DevOnBus->ID >= W25Q256)
	{
		cmd[i++] = (0x12);
		cmd[i++] = ((address & 0xFF000000) >> 24);
	}
	else
	{
		cmd[i++] = (0x02);
	}
	cmd[i++] = ((address & 0xFF0000) >> 16);
	cmd[i++] = ((address & 0xFF00) >> 8);
	cmd[i++] = (address & 0xFF);

	for(int j=0;j<len;j++)
		cmd[i++] = data[j];

	spi->SPI_WriteRead(spi->param,cmd,i,NULL,0);
	
	FLASH_WaitForWriteEnd(Param);
}

void FLASH_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	uint32_t page_size = spi->DevOnBus->PageSize;

	NorDB_sem_Lock(&spi->xSemaphore);	
	/*add Sector Offset*/
	address += FL->StartOffset;

	debug_info("w25qxx Write at Address:%d, %d Bytes\r\n", address, len);

    uint32_t start_page_offset = address % page_size;
    uint32_t start_page = address / page_size;
    uint32_t start_page_wbyte = (len+start_page_offset)<page_size ?  len:(page_size - (address - (start_page*page_size)));
    uint32_t remaining_byte = len - start_page_wbyte;
    uint32_t complete_page = remaining_byte / page_size;
    uint32_t Offset_end_Page = remaining_byte % page_size;
    
	uint8_t *dptr = data;
    if(start_page_wbyte)
    {
        debug_info("Write %i Byte in page %i from adr %i\r\n",start_page_wbyte, start_page,start_page_offset);
		FLASH_WritePage(Param,start_page * page_size + start_page_offset,dptr, start_page_wbyte);
		dptr+=start_page_wbyte;
    }
    
    for(int i=0;i<complete_page;i++)
    {
        debug_info("Write page(255 byte) in page %i\r\n",start_page+1+i);
		FLASH_WritePage(Param,(start_page+1+i) * page_size ,dptr, page_size);
		dptr+=page_size;
    }
    
    if(Offset_end_Page)
    {
        debug_info("Write %i Byte in page %i\r\n",Offset_end_Page,start_page+1+complete_page);
		FLASH_WritePage(Param,(start_page+1+complete_page) * page_size ,dptr, Offset_end_Page);
    }

	debug_info("w25qxx Write done.\r\n");
	NorDB_sem_Unlock(&spi->xSemaphore);
}

uint8_t FLASH_flashCheck(void*Param)
{	
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	SpiDev_t  *dev = spi->DevOnBus;

	return dev->ID != UnkownFlash;
}

const char *FLASH_DriverName(void*Param)
{
	static const char DName[] = "eFlash";
	return DName;
}

NorDB_HWLayer *FlashDB_Init(uint16_t StartSector,uint16_t TotalSector,SpiBus_t *spi)
{
	if(!spi) return NULL;

	NorDB_HWLayer *FlashLL = nordb_malloc(sizeof(NorDB_HWLayer));
	if(FlashLL==NULL)	return NULL;

	/*Check Device on Bus*/
	if(!spi->DevOnBus)
	{
		/*Searching bus for Flash Device*/
		spi->DevOnBus = nordb_malloc(sizeof(SpiDev_t));
		if(!spi->DevOnBus)
		{
			nordb_free(FlashLL);
			return NULL;
		}

		/*init Semaphore*/
		if(!NorDB_sem_init(&spi->xSemaphore))
		{
			Flashll_Del(FlashLL);
			return NULL;
		}

		if(!Searching_ForDevice(spi))
		{
			Flashll_Del(FlashLL);
			nordb_free(spi->DevOnBus);
			return NULL;
		}
	}	


	FlashDev_t *FL = nordb_malloc(sizeof(FlashDev_t));
	if(FL==NULL)
	{
		nordb_free(FlashLL);
		return NULL;
	}

	/*init sema*/
	if(!NorDB_sem_init(&FlashLL->sema))
	{
		Flashll_Del(FlashLL);
		return NULL;
	}
	FL->SectorSize  = spi->DevOnBus->SectorSize;
	FL->SPI 	    = spi;
	FL->StartOffset = StartSector * FL->SectorSize;
	FL->Total_Size  = FL->SectorSize * TotalSector;

	FlashLL->SectorSize 	= FL->SectorSize;
	FlashLL->SectorNumber	= TotalSector;
	FlashLL->Param			= FL;
	FlashLL->Erase 			= FLASH_Erase;
	FlashLL->SectorErace	= FLASH_EraseSector;
	FlashLL->WriteBuffer	= FLASH_WriteBuffer;
	FlashLL->ReadBuffer		= FLASH_ReadBuffer;
	FlashLL->DriverCheck	= FLASH_flashCheck;
	FlashLL->DriverName		= FLASH_DriverName;

	return FlashLL;
}

void Flashll_Del(NorDB_HWLayer *db)
{
	FlashDev_t *FL =(FlashDev_t*) db->Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;

	nordb_free(spi->DevOnBus);
	nordb_free(FL);
	nordb_free(db);
}




