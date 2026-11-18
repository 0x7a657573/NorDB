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

#define debug_info debug_log

#define FLASH_PAGE_SIZE	256

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

	spi->DevOnBus->PageSize = FLASH_PAGE_SIZE;
	spi->DevOnBus->SectorSize = 0x1000;
	spi->DevOnBus->SectorCount = spi->DevOnBus->BlockCount * 16;
	spi->DevOnBus->PageCount = (spi->DevOnBus->SectorCount * spi->DevOnBus->SectorSize) / spi->DevOnBus->PageSize;
	spi->DevOnBus->BlockSize = spi->DevOnBus->SectorSize * 16;
	
	debug_info("Flash Page Size: %d Bytes\r\n", spi->DevOnBus->PageSize);
	debug_info("Flash Page Count: %u\r\n", spi->DevOnBus->PageCount);
	debug_info("Flash Sector Size: %u Bytes\r\n", spi->DevOnBus->SectorSize);
	debug_info("Flash Sector Count: %u\r\n", spi->DevOnBus->SectorCount);
	debug_info("Flash Block Size: %u Bytes\r\n", spi->DevOnBus->BlockSize);
	debug_info("Flash Block Count: %u\r\n", spi->DevOnBus->BlockCount);
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

/*false when the chip is still busy after NORDB_FLASH_BUSY_MAX_POLL status reads*/
bool FLASH_WaitForWriteEnd(void *Param)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	uint8_t  StatusRegister1 = 0;
	uint8_t  cmd[8] = {0};
	cmd[0] = 0x05;

	//debug_info("w25qxx wait for flash ready\r\n");
	for(uint32_t n=0; n<NORDB_FLASH_BUSY_MAX_POLL; n++)
	{
		spi->SPI_WriteRead(spi->param,cmd,1,&StatusRegister1,1);
		if((StatusRegister1 & 0x01) == 0x00)
			return true;
	}
	err_log("w25qxx busy timeout after %u polls\r\n", NORDB_FLASH_BUSY_MAX_POLL);
	return false;
}

bool FLASH_EraseSector(void *Param,uint32_t SectorAddr)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	/*never touch a neighbouring partition*/
	if(SectorAddr > FL->Total_Size || FL->SectorSize > FL->Total_Size - SectorAddr)
		return false;
	NorDB_sem_Lock(&spi->xSemaphore);
	/*add Sector Offset*/
	SectorAddr += FL->StartOffset;
	debug_info("w25qxx EraseSector %u Begin...\r\n", SectorAddr);
	
	if(!FLASH_WaitForWriteEnd(Param))
	{
		NorDB_sem_Unlock(&spi->xSemaphore);
		return false;
	}
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
	
	bool ok = FLASH_WaitForWriteEnd(Param);

	debug_info("w25qxx EraseSector done.\r\n");
	NorDB_sem_Unlock(&spi->xSemaphore);
	return ok;
}

bool FLASH_ReadBuffer(void*Param, uint32_t ReadAddr, uint8_t *pBuffer,  uint16_t NumByteToRead)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	/*never touch a neighbouring partition*/
	if(ReadAddr > FL->Total_Size || NumByteToRead > FL->Total_Size - ReadAddr)
		return false;
	NorDB_sem_Lock(&spi->xSemaphore);
	/*add Sector Offset*/
	ReadAddr += FL->StartOffset;

	debug_info("w25qxx ReadBytes at Address:%u, %u Bytes\r\n", ReadAddr, NumByteToRead);

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
	return true;
}

static inline bool FLASH_WritePage(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;

	/*a page program can not cross a page*/
	if(len > FLASH_PAGE_SIZE)
		return false;

	FLASH_WriteEnable(Param);
	
	/*opcode + 32bit address + page data*/
	uint8_t cmd[1 + 4 + FLASH_PAGE_SIZE];
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
	
	return FLASH_WaitForWriteEnd(Param);
}

bool FLASH_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	/*never touch a neighbouring partition*/
	if(address > FL->Total_Size || len > FL->Total_Size - address)
		return false;
	uint32_t page_size = spi->DevOnBus->PageSize;

	NorDB_sem_Lock(&spi->xSemaphore);	
	/*add Sector Offset*/
	address += FL->StartOffset;

	debug_info("w25qxx Write at Address:%u, %u Bytes\r\n", address, len);

    uint32_t start_page_offset = address % page_size;
    uint32_t start_page = address / page_size;
    uint32_t start_page_wbyte = (len+start_page_offset)<page_size ?  len:(page_size - (address - (start_page*page_size)));
    uint32_t remaining_byte = len - start_page_wbyte;
    uint32_t complete_page = remaining_byte / page_size;
    uint32_t Offset_end_Page = remaining_byte % page_size;
    
	uint8_t *dptr = data;
	bool ok = true;
    if(start_page_wbyte)
    {
        debug_info("Write %u Byte in page %u from adr %u\r\n",start_page_wbyte, start_page,start_page_offset);
		ok = FLASH_WritePage(Param,start_page * page_size + start_page_offset,dptr, start_page_wbyte);
		dptr+=start_page_wbyte;
    }
    
    for(int i=0;ok && i<complete_page;i++)
    {
        debug_info("Write page(255 byte) in page %u\r\n",start_page+1+i);
		ok = FLASH_WritePage(Param,(start_page+1+i) * page_size ,dptr, page_size);
		dptr+=page_size;
    }
    
    if(ok && Offset_end_Page)
    {
        debug_info("Write %u Byte in page %u\r\n",Offset_end_Page,start_page+1+complete_page);
		ok = FLASH_WritePage(Param,(start_page+1+complete_page) * page_size ,dptr, Offset_end_Page);
    }

	debug_info("w25qxx Write done.\r\n");
	NorDB_sem_Unlock(&spi->xSemaphore);
	return ok;
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
	if(!spi || TotalSector==0) return NULL;

	bool NewBus = false;
	NorDB_HWLayer *FlashLL = nordb_malloc(sizeof(NorDB_HWLayer));
	if(FlashLL==NULL)	return NULL;
	nordb_memset(FlashLL,0,sizeof(NorDB_HWLayer));

	FlashDev_t *FL = nordb_malloc(sizeof(FlashDev_t));
	if(FL==NULL)
	{
		nordb_free(FlashLL);
		return NULL;
	}
	nordb_memset(FL,0,sizeof(FlashDev_t));

	/*Check Device on Bus*/
	if(!spi->DevOnBus)
	{
		/*Searching bus for Flash Device*/
		spi->DevOnBus = nordb_malloc(sizeof(SpiDev_t));
		if(!spi->DevOnBus)
			goto err_free;
		nordb_memset(spi->DevOnBus,0,sizeof(SpiDev_t));

		/*init Semaphore*/
		if(!NorDB_sem_init(&spi->xSemaphore))
		{
			nordb_free(spi->DevOnBus);
			spi->DevOnBus = NULL;
			goto err_free;
		}
		NewBus = true;

		if(!Searching_ForDevice(spi))
			goto err_bus;
	}

	if(((uint32_t)StartSector + TotalSector) > spi->DevOnBus->SectorCount)
		goto err_bus;

	/*init sema*/
	if(!NorDB_sem_init(&FlashLL->sema))
		goto err_bus;

	FL->SectorSize  = spi->DevOnBus->SectorSize;
	FL->SPI 	    = spi;
	FL->StartOffset = StartSector * FL->SectorSize;
	FL->Total_Size  = FL->SectorSize * TotalSector;

	FlashLL->SectorSize 	= FL->SectorSize;
	FlashLL->SectorNumber	= TotalSector;
	FlashLL->Param			= FL;
	FlashLL->SectorErace	= FLASH_EraseSector;
	FlashLL->WriteBuffer	= FLASH_WriteBuffer;
	FlashLL->ReadBuffer		= FLASH_ReadBuffer;
	FlashLL->DriverCheck	= FLASH_flashCheck;
	FlashLL->DriverName		= FLASH_DriverName;

	spi->RefCount++;
	return FlashLL;

err_bus:
	/*undo the bus setup done by this call only*/
	if(NewBus)
	{
		NorDB_sem_destroy(&spi->xSemaphore);
		nordb_free(spi->DevOnBus);
		spi->DevOnBus = NULL;
	}
err_free:
	nordb_free(FL);
	nordb_free(FlashLL);
	return NULL;
}

void Flashll_Del(NorDB_HWLayer *db)
{
	if(db==NULL)	return;

	FlashDev_t *FL =(FlashDev_t*) db->Param; 
	if(FL)
	{
		SpiBus_t *spi =(SpiBus_t*) FL->SPI;
		if(spi->RefCount>0)
			spi->RefCount--;
		if(spi->RefCount==0 && spi->DevOnBus)
		{
			NorDB_sem_destroy(&spi->xSemaphore);
			nordb_free(spi->DevOnBus);
			spi->DevOnBus = NULL;
		}
		nordb_free(FL);
	}
	NorDB_sem_destroy(&db->sema);
	nordb_free(db);
}
