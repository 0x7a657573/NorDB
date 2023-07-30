/*
 ============================================================================
 Name        : main.c
 Author      : Mohammad Mazarei
 Version     :
 Copyright   : Copyright 2020 Mohammad Mazarei This program is free software
 Description : Test app for NorDB in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <NorDB.h>
#include <ll/SerialNorFlash_ll.h>
#include "ch341.h"

typedef struct  __attribute__((__packed__))
{
	uint8_t  dummy[11];
	uint32_t checksum;
}dummy_t;

bool FullFill_Tetst(NorDB_t *DB);
bool RoundRobin_Test(NorDB_t *DB);
bool OverWrite_Test(NorDB_t *DB);

int spiFlashTest(void);

int main(void)
{
	/*add record to DB*/
	CH341DeviceInit();
	spiFlashTest();
	return EXIT_SUCCESS;
}

void get_RandomRecord(dummy_t *rec)
{
	rec->checksum = 0;
	for(int i=0;i<11;i++)
	{
		rec->dummy[i] = rand() & 0xFF;
		rec->checksum += rec->dummy[i];
	}
}

bool check_bummyRecord(dummy_t *rec)
{
	uint32_t sum = 0;
	for(int i=0;i<11;i++)
	{
		sum += rec->dummy[i];
	}
	return sum==rec->checksum;
}


bool FullFill_Tetst(NorDB_t *DB)
{
	dummy_t temp;

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;

	uint32_t Used_Record = NorDB_Get_TotalUnreadRecord(DB);
	printf("--->NorDB Full Fill Test\n");

	if(Used_Record)
	{
		
		printf("Have %d record try empty DB\n",Used_Record);
		uint32_t add_record = Total_Capacity - Used_Record;
		for(int i=0;i<add_record;i++)
		{
			/*get random record*/
			get_RandomRecord(&temp);
			NorDB_AddRecord(DB, &temp);
		}

		for(int i=0;i<Total_Capacity;i++)
		{
			NorDB_ReadRecord(DB, &temp);
		}
	}

	/*try fill DB*/
	uint32_t LastUpdateSector = -1;
	for(int i=0;i<Total_Capacity;i++)
	{
		/*get random record*/
		get_RandomRecord(&temp);
		uint32_t x = NorDB_AddRecord(DB, &temp);
		if(x==0)
		{
			printf("Error to add Rec %i\n",i);
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("Sector %d Fill Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}
	printf("incert %d Record in DB Completed\n\n",Total_Capacity);

	/*Try ReadBack*/
	LastUpdateSector = -1;
	for(int i=0;i<Total_Capacity;i++)
	{
		/*get random record*/
		uint32_t x = NorDB_ReadRecord(DB, &temp);
		if(x==0)
		{
			printf("Error to Read Rec %i\n",i);
			return false;
		}

		if(check_bummyRecord(&temp)==false)
		{
			printf("Read Reocrd not Correct!\n");
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("Sector %d Read Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}
	printf("Read Back %d Record Correctly :)\n\n",Total_Capacity);

	return true;
}


bool RoundRobin_Test(NorDB_t *DB)
{
	dummy_t temp;

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;

	uint32_t Used_Record = NorDB_Get_TotalUnreadRecord(DB);
	printf("--->Round Robin Test NorDB\n");
	

	/*be sure empty database*/
	if(Used_Record)
	{
		printf("Have %d record try empty DB\n",Used_Record);
		uint32_t add_record = Total_Capacity - Used_Record;
		for(int i=0;i<add_record;i++)
		{
			/*get random record*/
			get_RandomRecord(&temp);
			NorDB_AddRecord(DB, &temp);
		}

		for(int i=0;i<Total_Capacity;i++)
		{
			NorDB_ReadRecord(DB, &temp);
		}
	}

	uint32_t lastSectorUse = -1;
	do
	{
		/*try fill OneSector*/
		uint32_t WriteSector = -1;
		for(int i=0;i<Record_Sector;i++)
		{
			/*get random record*/
			get_RandomRecord(&temp);
			uint32_t x = NorDB_AddRecord(DB, &temp);
			if(x==0)
			{
				printf("Error to add Rec %i\n",i);
				return false;
			}

			uint32_t useSector = (x/Sector_Size);
			if(WriteSector!=useSector)
			{
				printf("Sector %d Fill Completed\n",useSector);
				WriteSector = useSector;
			}
		}

		/*Try ReadBack*/
		uint32_t readBack = -1;
		for(int i=0;i<Record_Sector;i++)
		{
			/*get random record*/
			uint32_t x = NorDB_ReadRecord(DB, &temp);
			if(x==0)
			{
				printf("Error to Read Rec %i\n",i);
				return false;
			}

			if(check_bummyRecord(&temp)==false)
			{
				printf("Read Reocrd not Correct!\n");
				return false;
			}

			uint32_t useSector = (x/Sector_Size);
			if(readBack!=useSector)
			{
				printf("Sector %d Read Completed\n",useSector);
				readBack = useSector;
			}
		}

		if(lastSectorUse==-1)
			lastSectorUse = WriteSector;
		else if(lastSectorUse == WriteSector)
		{
			printf("Round Robin Test Failed\n");	
			return false;
		}
		else
		{
			break;
		}
	} while (1);
	printf("Round Robin Correctly Work:)\n\n",Total_Capacity);

	return true;
}

bool OverWrite_Test(NorDB_t *DB)
{
	dummy_t temp;

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;
	uint32_t Used_Record = NorDB_Get_TotalUnreadRecord(DB);
	printf("--->NorDB Over Write Test\n");

	/*be sure empty database*/
	if(Used_Record)
	{
		
		printf("Have %d record try empty DB\n",Used_Record);
		uint32_t add_record = Total_Capacity - Used_Record;
		for(int i=0;i<add_record;i++)
		{
			/*get random record*/
			get_RandomRecord(&temp);
			NorDB_AddRecord(DB, &temp);
		}

		for(int i=0;i<Total_Capacity;i++)
		{
			NorDB_ReadRecord(DB, &temp);
		}
	}

	uint32_t LastUpdateSector = -1;
	for(int i=0;i<Total_Capacity;i++)
	{
		/*get random record*/
		get_RandomRecord(&temp);
		uint32_t x = NorDB_AddRecord(DB, &temp);
		if(x==0)
		{
			printf("Error to add Rec %i\n",i);
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("Sector %d Fill Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}
	printf("incert %d Record in DB Completed\n\n",Total_Capacity);
	printf("Add One Record More!\n");
	/*try add another record*/
	get_RandomRecord(&temp);
	uint32_t x = NorDB_AddRecord(DB, &temp);
	if(x!=0)
	{
		printf("Over Write Test Failed %i\n",x);
		return false;
	}
	printf("Every Think is OK.\n\n");
	
	/*Try ReadBack*/
	LastUpdateSector = -1;
	for(int i=0;i<Total_Capacity;i++)
	{
		/*get random record*/
		uint32_t x = NorDB_ReadRecord(DB, &temp);
		if(x==0)
		{
			printf("Error to Read Rec %i\n",i);
			return false;
		}

		if(check_bummyRecord(&temp)==false)
		{
			printf("Read Reocrd not Correct!\n");
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("Sector %d Read Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}

	printf("Over Write Protection Correctly Work:)\n\n",Total_Capacity);
	return true;
}

void xSPI_WriteRead(void *param,uint8_t *out,uint16_t wlen,uint8_t *in,uint16_t rlen)
{
	CH341ChipSelect(0, true);
	CH341WriteSPI(out, wlen);
	CH341ReadSPI(in, rlen);
	CH341ChipSelect(0, false);
}

int spiFlashTest(void)
{
	SpiBus_t xSPI;
	void (*SPI_WriteRead)(void *param,uint8_t *out,uint16_t wlen,uint8_t *in,uint16_t rlen);
	xSPI.param = NULL;
	xSPI.DevOnBus = NULL;
	xSPI.SPI_WriteRead = xSPI_WriteRead;
	NorDB_HWLayer *Flash_HW = FlashDB_Init(0, 4,&xSPI);
			
	dummy_t temp;
	srand(time(NULL));
	
	NorDB_t *DB = NorDB(Flash_HW,sizeof(dummy_t));
	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;

	printf("************************************\n");
	printf("***********SPI Flash Test***********\n");
	printf("************************************\n");

	printf("NorDB Tests\n\tSectors:%d\n",Sector_Number);
	printf("\tRecordPerSector:%d\n",Record_Sector);
	printf("\tTotal Capacity:%d\n",Total_Capacity);

	printf("***************************************\n");
	printf("Sector_Size:%d\n",Sector_Size);
	printf("Sector_Number:%d\n",Sector_Number);
	printf("Record_NumberInSector:%d\n",Record_Sector);
	printf("***************************************\n");

	/*fill test*/
	if(!FullFill_Tetst(DB))
	{
		printf("Fill Test Not Complete\n");
		Flashll_Del(Flash_HW);
		return EXIT_FAILURE;
	}

	/*Round Robin test*/
	if(!RoundRobin_Test(DB))
	{
		printf("Round Robin Test Not Complete\n");
		Flashll_Del(Flash_HW);
		return EXIT_FAILURE;
	}

	/*OverWrite test*/
	if(!OverWrite_Test(DB))
	{
		printf("Over Write Test Not Complete\n");
		Flashll_Del(Flash_HW);
		return EXIT_FAILURE;
	}

	Flashll_Del(Flash_HW);
}

