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
#include "TestRoutines.h"


void xSPI_WriteRead(void *param,uint8_t *out,uint16_t wlen,uint8_t *in,uint16_t rlen)
{
	CH341ChipSelect(0, true);
	CH341WriteSPI(out, wlen);
	CH341ReadSPI(in, rlen);
	CH341ChipSelect(0, false);
}

int main(void)
{
	/*add record to DB*/
	CH341DeviceInit();
	
	SpiBus_t xSPI;
	void (*SPI_WriteRead)(void *param,uint8_t *out,uint16_t wlen,uint8_t *in,uint16_t rlen);
	xSPI.param = NULL;
	xSPI.DevOnBus = NULL;
	xSPI.SPI_WriteRead = xSPI_WriteRead;
	NorDB_HWLayer *Flash_HW = FlashDB_Init(0, 4,&xSPI);
			
	NorDB_t *DB = NorDB(Flash_HW, GetDummyRecordSize());
	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;

	printf("NorDB Tests\n\tSectors:%d\n",Sector_Number);
	printf("\tRecordPerSector:%d\n",Record_Sector);
	printf("\tTotal Capacity:%d\n",Total_Capacity);

	printf("***************************************\n");
	printf("Sector_Size:%d\n",Sector_Size);
	printf("Sector_Number:%d\n",Sector_Number);
	printf("Record_NumberInSector:%d\n",Record_Sector);
	printf("***************************************\n");

	printf("\n**********FLASH TEST**********\n");
	
	RunTest(FullFill_Test, DB, 1);
	RunTest(RoundRobin_Test, DB, 1);
	RunTest(OverWrite_Test, DB, 1);
	RunTest(WriteRead_Time_Test, DB, 100);
	RunTest(DeleteDB_Test, DB, 100);
	RunTest(Clear_Test, DB, 100);

	return EXIT_SUCCESS;
}




