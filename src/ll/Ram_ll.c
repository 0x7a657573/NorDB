/*
 * Ram_ll.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */

#include "ll/Ram_ll.h"

void RAMll_Erase(void *Param)
{
	Ramll_t *ptr = Param;

	nordb_memset(ptr->Buffer,0xFF,ptr->Total_Size);
}

void RAMll_SectorErace(void*Param,uint32_t address)
{
	Ramll_t *ptr = Param;

	uint16_t Sector_Number  = address / ptr->Sector_Size;
	if(Sector_Number < ptr->Total_Sector)
		nordb_memset(ptr->Buffer+(Sector_Number*ptr->Sector_Size),0xFF,ptr->Sector_Size);
}


void RAMll_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	Ramll_t *ptr = Param;

	if((address+len) < ptr->Total_Size)
	{
		nordb_memcpy(&ptr->Buffer[address],data,len);
		//printf("wRAM (%d)->%d\n",address,len);
	}
}

void RAMll_ReadBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	Ramll_t *ptr = Param;

	if((address+len) < ptr->Total_Size)
	{
		nordb_memcpy(data,&ptr->Buffer[address],len);
		//printf("rRAM (%d)->%d\n",address,len);
	}
}

uint8_t RAMll_flashCheck(void*Param)
{
	//RamBuffer *ram = (RamBuffer*)Param;
	return 1;
}

const char *RAMll_DriverName(void*Param)
{
	static const char DName[] = "iRam";
	return DName;
}


NorDB_HWLayer *Ramll_Init(uint16_t SectorSize,uint16_t TotalSector)
{
	NorDB_HWLayer *Ramhw = nordb_malloc(sizeof(NorDB_HWLayer));
	if(Ramhw==NULL)	return NULL;

	uint32_t total_Size = TotalSector *  SectorSize;
	Ramll_t *Ramsw = nordb_malloc(sizeof(Ramll_t));
	if(Ramsw==NULL)
	{
		nordb_free(Ramhw);
		return NULL;
	}

	Ramsw->Sector_Size = SectorSize;
	Ramsw->Total_Sector = TotalSector;
	Ramsw->Total_Size	= total_Size;
	Ramsw->Buffer = nordb_malloc(total_Size);
	if(Ramsw->Buffer==NULL)
	{
		nordb_free(Ramsw);
		nordb_free(Ramhw);
		return NULL;
	}

	/*init sema*/
	if(!NorDB_sem_init(&Ramhw->sema))
	{
		Ramll_Del(Ramhw);
		return NULL;
	}

	Ramhw->SectorSize 		= SectorSize;
	Ramhw->SectorNumber		= TotalSector;
	Ramhw->Param			= Ramsw;
	Ramhw->Erase 			= RAMll_Erase;
	Ramhw->SectorErace		= RAMll_SectorErace;
	Ramhw->WriteBuffer		= RAMll_WriteBuffer;
	Ramhw->ReadBuffer		= RAMll_ReadBuffer;
	Ramhw->DriverCheck		= RAMll_flashCheck;
	Ramhw->DriverName		= RAMll_DriverName;

	return Ramhw;
}


void Ramll_Del(NorDB_HWLayer *db)
{
	nordb_free((((Ramll_t*)db->Param)->Buffer));
	nordb_free(db->Param);
	nordb_free(db);
}
