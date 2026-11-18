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


bool xSPI_WriteRead(void *param,uint8_t *out,uint16_t wlen,uint8_t *in,uint16_t rlen)
{
	bool ok = CH341ChipSelect(0, true);
	if(ok)
		ok = CH341WriteSPI(out, wlen);
	if(ok)
		ok = CH341ReadSPI(in, rlen);
	bool selected = CH341ChipSelect(0, false);
	return ok && selected;
}

int main(int argc, char **argv)
{
	/*add record to DB*/
	if (!CH341DeviceInit())
	{
		printf("Error: CH341DeviceInit failed\n");
		return EXIT_FAILURE;
	}
	srand(time(NULL));
	
	SpiBus_t xSPI = {0};
	bool (*SPI_WriteRead)(void *param,uint8_t *out,uint16_t wlen,uint8_t *in,uint16_t rlen);
	xSPI.param = NULL;
	xSPI.SPI_WriteRead = xSPI_WriteRead;

	uint32_t partition = 0;
	uint32_t sector_count = 4;
	if (argc >= 2)
		partition = (uint32_t)strtoul(argv[1], NULL, 0);
	if (argc >= 3)
		sector_count = (uint32_t)strtoul(argv[2], NULL, 0);

	NorDB_HWLayer *Flash_HW = FlashDB_Init(partition, sector_count, &xSPI);
	NorDB_t *DB = NULL;
	int result;
	if (!Flash_HW)
	{
		printf("Error: FlashDB_Init failed\n");
		result = EXIT_FAILURE;
		goto cleanup;
	}
			
	DB = NorDB(Flash_HW, GetDummyRecordSize());
	if (!DB)
	{
		printf("Error: NorDB init failed\n");
		result = EXIT_FAILURE;
		goto cleanup;
	}
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
	
	/* stride 32: about 16 positions * 512 writes * 5 ms = 41 s. */
	RunFlashTests(DB, 32);

	result = TestSummary();

cleanup:
	NorDB_Destroy(DB);
	Flashll_Del(Flash_HW);
	CH341DeviceRelease();
	return result;
}




