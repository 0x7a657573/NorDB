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
#include <ll/Ram_ll.h>
#include "TestRoutines.h"

int main(int argc, char **argv)
{
	srand(time(NULL));

	uint32_t sector_size = 4096;
	uint32_t sector_count = 4;
	if (argc >= 2)
		sector_size = (uint32_t)strtoul(argv[1], NULL, 0);
	if (argc >= 3)
		sector_count = (uint32_t)strtoul(argv[2], NULL, 0);

	NorDB_HWLayer *Ram_Hw = Ramll_Init(sector_size, sector_count);	
	if (!Ram_Hw)
	{
		printf("Error: Ramll_Init failed\n");
		return EXIT_FAILURE;
	}

	NorDB_t *DB = NorDB(Ram_Hw, GetDummyRecordSize());
	if (!DB)
	{
		printf("Error: NorDB init failed\n");
		return EXIT_FAILURE;
	}

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;
	printf("NorDB Tests\n\tSectors:%d\n",Sector_Number);
	printf("\tRecordPerSector:%d\n",Record_Sector);
	printf("\tTotal Capacity:%d\n",Total_Capacity);

	printf("\n**********RAM TEST**********\n");
	RunTest(FullFill_Test, DB, 1);
	RunTest(RoundRobin_Test, DB, 1);
	RunTest(OverWrite_Test, DB, 1);
	RunTest(WriteRead_Time_Test, DB, 10);
	RunTest(CRC_DetectsCorruption_Test, DB, 0);
	RunTest(DeleteDB_Test, DB, 10);
	RunTest(Clear_Test, DB, 100);
	RunTest(ReadEmpty_Test, DB, 0);
	
	return EXIT_SUCCESS;
}
