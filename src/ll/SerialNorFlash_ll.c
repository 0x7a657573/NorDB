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
#define SFDP_READ_OPCODE	0x5A
#define SFDP_BFPT_ID		0xFF00
#define SFDP_4BAIT_ID		0xFF84

static inline uint32_t FLASH_WriteCommand(uint8_t *cmd,uint8_t opcode,uint8_t addrbytes,uint32_t address)
{
	uint32_t i = 0;
	cmd[i++] = opcode;
	for(uint8_t n = addrbytes; n > 0; n--)
		cmd[i++] = (address >> ((n - 1) * 8));
	return i;
}

typedef enum
{
	SFDP_NOT_FOUND,
	SFDP_CONFIGURED,
	SFDP_UNUSABLE,
	SFDP_IO_ERROR
} SfdpResult_t;

static bool SFDP_Read(SpiBus_t *spi,uint32_t address,uint8_t *data,uint16_t length)
{
	uint8_t cmd[5] = {SFDP_READ_OPCODE,0,0,0,0};
	cmd[1] = (uint8_t)(address >> 16);
	cmd[2] = (uint8_t)(address >> 8);
	cmd[3] = (uint8_t)address;
	return spi->SPI_WriteRead(spi->param,cmd,sizeof(cmd),data,length);
}

static uint32_t SFDP_U32(const uint8_t *data)
{
	return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
		((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static bool SFDP_IsPowerOfTwo(uint32_t value)
{
	return value != 0 && (value & (value - 1)) == 0;
}

static bool SFDP_FindParameter(SpiBus_t *spi,uint8_t parameter_count,uint16_t id,
	uint8_t *length,uint32_t *address,bool *found)
{
	uint8_t header[8];
	*found = false;
	for(uint8_t n = 0; n <= parameter_count; n++)
	{
		if(!SFDP_Read(spi,8U + 8U * n,header,sizeof(header)))
			return false;
		if(((uint16_t)header[0] | ((uint16_t)header[7] << 8)) == id)
		{
			*length = header[3];
			*address = (uint32_t)header[4] | ((uint32_t)header[5] << 8) |
				((uint32_t)header[6] << 16);
			*found = true;
			return true;
		}
	}
	return true;
}

static SfdpResult_t SFDP_Configure(SpiBus_t *spi)
{
	uint8_t bfpt_length;
	uint8_t parameter_length;
	uint32_t parameter_address;
	uint8_t bfpt[36] = {0};
	uint8_t dword11_data[4];
	uint8_t bait_data[8];
	uint8_t erase_type = 0;
	uint8_t erase_opcode = 0;
	uint32_t dword1;
	uint32_t dword2;
	uint32_t dword8;
	uint32_t dword9;
	uint32_t dword11;
	uint32_t bait1;
	uint32_t bait2;
	uint32_t density_bits;
	uint32_t size;
	uint16_t page_size = FLASH_PAGE_SIZE;
	uint8_t parameter_count;
	bool has_bait;

	if(!SFDP_Read(spi,0,bfpt,8))
		return SFDP_IO_ERROR;
	if(bfpt[0] != 'S' || bfpt[1] != 'F' || bfpt[2] != 'D' ||
		bfpt[3] != 'P' || bfpt[5] != 1)
		return SFDP_NOT_FOUND;
	parameter_count = bfpt[6];
	bool parameter_found;
	if(!SFDP_FindParameter(spi,parameter_count,SFDP_BFPT_ID,
		&bfpt_length,&parameter_address,&parameter_found))
		return SFDP_IO_ERROR;
	if(!parameter_found || bfpt_length < 9)
		return SFDP_NOT_FOUND;
	if(bfpt_length > 16)
		bfpt_length = 16;
	if(!SFDP_Read(spi,parameter_address,bfpt,sizeof(bfpt)))
		return SFDP_IO_ERROR;
	dword1 = SFDP_U32(bfpt);
	dword2 = SFDP_U32(&bfpt[4]);
	dword8 = SFDP_U32(&bfpt[28]);
	dword9 = SFDP_U32(&bfpt[32]);

	if(dword2 & 0x80000000U)
	{
		uint32_t exponent = dword2 & 0x7FFFFFFFU;
		if(exponent > 31)
			return SFDP_NOT_FOUND;
		density_bits = (uint32_t)1U << exponent;
	}
	else
		density_bits = dword2 + 1U;
	if((density_bits & 7U) != 0)
		return SFDP_NOT_FOUND;
	size = density_bits / 8U;
	if(size < 64U * 1024U || size > 256U * 1024U * 1024U ||
		!SFDP_IsPowerOfTwo(size))
		return SFDP_NOT_FOUND;

	if((dword1 & 3U) != 1U)
	{
		err_log("SFDP flash has no uniform 4 KB erase support\r\n");
		return SFDP_UNUSABLE;
	}
	for(uint8_t n = 0; n < 4; n++)
	{
		uint32_t erase_dword = n < 2 ? dword8 : dword9;
		uint8_t shift = (n % 2) * 16;
		uint8_t erase_size = (uint8_t)(erase_dword >> shift);
		uint8_t opcode = (uint8_t)(erase_dword >> (shift + 8));
		if(erase_size == 12 && opcode != 0)
		{
			erase_type = n + 1;
			erase_opcode = opcode;
			break;
		}
	}
	if(erase_type == 0)
	{
		err_log("SFDP flash has no 4 KB erase type in DWORD8/9\r\n");
		return SFDP_UNUSABLE;
	}

	uint8_t address_mode = (uint8_t)((dword1 >> 17) & 3U);
	if(bfpt_length >= 11)
	{
		if(!SFDP_Read(spi,parameter_address + 40,dword11_data,sizeof(dword11_data)))
			return SFDP_IO_ERROR;
		dword11 = SFDP_U32(dword11_data);
		uint8_t exponent = (uint8_t)((dword11 >> 4) & 0x0F);
		if(exponent >= 1 && exponent < 16)
			page_size = (uint16_t)(1U << exponent);
	}
	spi->DevOnBus->PageSize = page_size < FLASH_PAGE_SIZE ? page_size : FLASH_PAGE_SIZE;
	if(!SFDP_FindParameter(spi,parameter_count,SFDP_4BAIT_ID,
		&parameter_length,&parameter_address,&has_bait))
		return SFDP_IO_ERROR;
	has_bait = has_bait && parameter_length >= 2;
	if(has_bait)
	{
		if(!SFDP_Read(spi,parameter_address,bait_data,sizeof(bait_data)))
			return SFDP_IO_ERROR;
		bait1 = SFDP_U32(bait_data);
		bait2 = SFDP_U32(&bait_data[4]);
	}

	spi->DevOnBus->ID = SfdpFlash;
	spi->DevOnBus->SectorSize = 0x1000;
	spi->DevOnBus->BlockSize = 0x10000;
	spi->DevOnBus->SectorCount = size / spi->DevOnBus->SectorSize;
	spi->DevOnBus->AddrBytes = 3;
	spi->DevOnBus->SectorEraseOpcode = erase_opcode;
	spi->DevOnBus->FastReadOpcode = 0x0B;
	spi->DevOnBus->PageProgramOpcode = 0x02;
	if(size > 16U * 1024U * 1024U || address_mode == 2)
	{
		if(address_mode == 2 && !has_bait)
		{
			err_log("SFDP flash is 4-byte-only without 4BAIT support\r\n");
			return SFDP_UNUSABLE;
		}
		bool four_byte_ok = has_bait && (bait1 & (1U << 1)) != 0 &&
			(bait1 & (1U << 6)) != 0 &&
			(bait1 & (1U << (8 + erase_type))) != 0 &&
			((bait2 >> ((erase_type - 1) * 8)) & 0xFFU) != 0;
		if(four_byte_ok)
		{
			spi->DevOnBus->AddrBytes = 4;
			spi->DevOnBus->SectorEraseOpcode =
				(uint8_t)(bait2 >> ((erase_type - 1) * 8));
			spi->DevOnBus->FastReadOpcode = 0x0C;
			spi->DevOnBus->PageProgramOpcode = 0x12;
		}
		else if(size > 16U * 1024U * 1024U)
		{
			err_log("SFDP flash has no usable 4-byte instructions; using first 16 MB\r\n");
			spi->DevOnBus->SectorCount = (16U * 1024U * 1024U) /
				spi->DevOnBus->SectorSize;
		}
		else
		{
			err_log("SFDP flash has no usable 4-byte instructions\r\n");
			return SFDP_UNUSABLE;
		}
	}
	spi->DevOnBus->BlockCount = spi->DevOnBus->SectorCount *
		spi->DevOnBus->SectorSize / spi->DevOnBus->BlockSize;
	spi->DevOnBus->PageCount = (spi->DevOnBus->SectorCount *
		spi->DevOnBus->SectorSize) / spi->DevOnBus->PageSize;
	return SFDP_CONFIGURED;
}

static bool FLASH_ConfigureFallback(SpiBus_t *spi)
{
	switch (spi->DevOnBus->JEDECID & 0x000000FF)
	{
	case 0x20: spi->DevOnBus->ID = W25Q512; spi->DevOnBus->BlockCount = 1024; break;
	case 0x19: spi->DevOnBus->ID = W25Q256; spi->DevOnBus->BlockCount = 512; break;
	case 0x18: spi->DevOnBus->ID = W25Q128; spi->DevOnBus->BlockCount = 256; break;
	case 0x17: spi->DevOnBus->ID = W25Q64; spi->DevOnBus->BlockCount = 128; break;
	case 0x16: spi->DevOnBus->ID = W25Q32; spi->DevOnBus->BlockCount = 64; break;
	case 0x15: spi->DevOnBus->ID = W25Q16; spi->DevOnBus->BlockCount = 32; break;
	case 0x14: spi->DevOnBus->ID = W25Q80; spi->DevOnBus->BlockCount = 16; break;
	case 0x13: spi->DevOnBus->ID = W25Q40; spi->DevOnBus->BlockCount = 8; break;
	case 0x12: spi->DevOnBus->ID = W25Q20; spi->DevOnBus->BlockCount = 4; break;
	case 0x11: spi->DevOnBus->ID = W25Q10; spi->DevOnBus->BlockCount = 2; break;
	default: return false;
	}
	spi->DevOnBus->AddrBytes = 3;
	spi->DevOnBus->SectorEraseOpcode = 0x20;
	spi->DevOnBus->FastReadOpcode = 0x0B;
	spi->DevOnBus->PageProgramOpcode = 0x02;
	if(spi->DevOnBus->ID == W25Q256 || spi->DevOnBus->ID == W25Q512)
	{
		spi->DevOnBus->AddrBytes = 4;
		spi->DevOnBus->SectorEraseOpcode = 0x21;
		spi->DevOnBus->FastReadOpcode = 0x0C;
		spi->DevOnBus->PageProgramOpcode = 0x12;
	}
	spi->DevOnBus->PageSize = FLASH_PAGE_SIZE;
	spi->DevOnBus->SectorSize = 0x1000;
	spi->DevOnBus->SectorCount = spi->DevOnBus->BlockCount * 16;
	spi->DevOnBus->PageCount = (spi->DevOnBus->SectorCount * spi->DevOnBus->SectorSize) /
		spi->DevOnBus->PageSize;
	spi->DevOnBus->BlockSize = spi->DevOnBus->SectorSize * 16;
	return true;
}

bool Searching_ForDevice(SpiBus_t *spi)
{
	uint8_t status = 0;
	uint8_t status_cmd[8] = {0x05};
	uint8_t wr_buff[32]={0x9f};
    uint8_t rd_buff[32]={0xff};
	bool ready = false;
	for(uint32_t n=0; n<NORDB_FLASH_BUSY_MAX_POLL; n++)
	{
		if(!spi->SPI_WriteRead(spi->param,status_cmd,1,&status,1))
			return false;
		if((status & 0x01) == 0x00)
		{
			spi->MayBeBusy = false;
			ready = true;
			break;
		}
	}
	if(!ready)
		return false;

	if(!spi->SPI_WriteRead(spi->param,wr_buff,1,rd_buff,3))
		return false;
	spi->DevOnBus->JEDECID = (rd_buff[0] << 16) | (rd_buff[1] << 8) | rd_buff[2]; //JEDEC ID
	
	SfdpResult_t sfdp = SFDP_Configure(spi);
	if(sfdp == SFDP_UNUSABLE || sfdp == SFDP_IO_ERROR)
		return false;
	if(sfdp == SFDP_NOT_FOUND && !FLASH_ConfigureFallback(spi))
		return false;
	
	debug_info("Flash Page Size: %d Bytes\r\n", spi->DevOnBus->PageSize);
	debug_info("Flash Page Count: %u\r\n", spi->DevOnBus->PageCount);
	debug_info("Flash Sector Size: %u Bytes\r\n", spi->DevOnBus->SectorSize);
	debug_info("Flash Sector Count: %u\r\n", spi->DevOnBus->SectorCount);
	debug_info("Flash Block Size: %u Bytes\r\n", spi->DevOnBus->BlockSize);
	debug_info("Flash Block Count: %u\r\n", spi->DevOnBus->BlockCount);
	debug_info("Flash Init Done\r\n");
	return true;
}

bool FLASH_WriteEnable(void *Param)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;
	//debug_info("w25qxx Write Enable\r\n");
	uint8_t  cmd[8] = {0};
	cmd[0] = 0x06;
	return spi->SPI_WriteRead(spi->param,cmd,1,NULL,0);
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
		if(!spi->SPI_WriteRead(spi->param,cmd,1,&StatusRegister1,1))
		{
			spi->MayBeBusy = true;
			return false;
		}
		if((StatusRegister1 & 0x01) == 0x00)
			return true;
	}
	err_log("w25qxx busy timeout after %u polls\r\n", NORDB_FLASH_BUSY_MAX_POLL);
	spi->MayBeBusy = true;
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
	spi->MayBeBusy = false;
	if(!FLASH_WriteEnable(Param))
	{
		NorDB_sem_Unlock(&spi->xSemaphore);
		return false;
	}

	uint8_t  cmd[8] = {0};
	uint32_t i = FLASH_WriteCommand(cmd,spi->DevOnBus->SectorEraseOpcode,spi->DevOnBus->AddrBytes,SectorAddr);
	if(!spi->SPI_WriteRead(spi->param,cmd,i,NULL,0))
	{
		spi->MayBeBusy = true;
		NorDB_sem_Unlock(&spi->xSemaphore);
		return false;
	}
	
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
	if(spi->MayBeBusy)
	{
		if(!FLASH_WaitForWriteEnd(Param))
		{
			NorDB_sem_Unlock(&spi->xSemaphore);
			return false;
		}
		spi->MayBeBusy = false;
	}
	/*add Sector Offset*/
	ReadAddr += FL->StartOffset;

	debug_info("w25qxx ReadBytes at Address:%u, %u Bytes\r\n", ReadAddr, NumByteToRead);

	uint32_t Index = 0;
	uint8_t  cmd[8] = {0};
	Index = FLASH_WriteCommand(cmd,spi->DevOnBus->FastReadOpcode,spi->DevOnBus->AddrBytes,ReadAddr);
	cmd[Index++] = (0);
	bool ok = spi->SPI_WriteRead(spi->param,cmd,Index,pBuffer,NumByteToRead);
	NorDB_sem_Unlock(&spi->xSemaphore);
	return ok;
}

static inline bool FLASH_WritePage(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	FlashDev_t *FL =(FlashDev_t*) Param; 
	SpiBus_t *spi =(SpiBus_t*) FL->SPI;

	/*a page program can not cross a page*/
	if(len > FLASH_PAGE_SIZE)
		return false;
	if(spi->MayBeBusy)
	{
		if(!FLASH_WaitForWriteEnd(Param))
			return false;
		spi->MayBeBusy = false;
	}

	if(!FLASH_WriteEnable(Param))
		return false;
	
	/*opcode + 32bit address + page data*/
	uint8_t cmd[1 + 4 + FLASH_PAGE_SIZE];
	uint32_t i = FLASH_WriteCommand(cmd,spi->DevOnBus->PageProgramOpcode,spi->DevOnBus->AddrBytes,address);

	for(int j=0;j<len;j++)
		cmd[i++] = data[j];

	if(!spi->SPI_WriteRead(spi->param,cmd,i,NULL,0))
	{
		spi->MayBeBusy = true;
		return false;
	}
	
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
		spi->MayBeBusy = true;

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
