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


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "../NorDB.h"

typedef struct
{
	uint8_t UniqID[8];
	uint16_t PageSize;
	uint32_t PageCount;
	uint32_t 
	SectorSize;
	uint32_t SectorCount;
	uint32_t BlockSize;
	uint32_t BlockCount;
	uint32_t CapacityInKiloByte;
	uint8_t StatusRegister1;
	uint8_t StatusRegister2;
	uint8_t StatusRegister3;
	uint8_t Lock;

}SF_t;

typedef struct
{
	uint16_t 		Sector_Size;
	uint16_t 		Total_Sector;
	uint32_t		Total_Size;
	uint8_t  		*Buffer;
}SpiFlashll_t;

typedef struct
{
	SpiFlashll_t 	*Param;
	SF_t			SF;
	
	bool	(*SpiChipSelect)(unsigned int cs, bool enable);
	bool	(*SpiStream)(const unsigned char *in, unsigned char *out, unsigned int size);
	bool	(*SpiWrite)(const unsigned char *in, unsigned int size);
	bool	(*SpiRead)(unsigned char *out, unsigned int size);

}SpiFlashll_Driver;

NorDB_HWLayer *SpiFlashll_Init(SpiFlashll_Driver *SpiHw,uint16_t SectorSize,uint16_t TotalSector);
void SpiFlash_Del(NorDB_HWLayer *db);


void SF_WaitForWriteEnd(SpiFlashll_Driver *SpiHw);
void SFQ_Read_Manifacture_Device_ID(SpiFlashll_Driver *SpiHw);
bool SF_Init(SpiFlashll_Driver *SpiHw);
void SF_EraseChip(SpiFlashll_Driver *SpiHw);
void SF_WriteByte(SpiFlashll_Driver *SpiHw, uint8_t pBuffer, uint32_t WriteAddr_inBytes);
void SF_WriteMoreByte(SpiFlashll_Driver *SpiHw, uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes);
void SF_Write(SpiFlashll_Driver *SpiHw, uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes);
void SF_ReadByte(SpiFlashll_Driver *SpiHw, unsigned char *pBuffer, uint32_t ReadAddr);
void SF_ReadMoreByte(SpiFlashll_Driver *SpiHw, uint8_t *recive, uint32_t len, uint32_t WriteAddr_inBytes);
void SF_Read(SpiFlashll_Driver *SpiHw, uint8_t *recive, uint32_t len, uint32_t ReadAddr_inBytes);
