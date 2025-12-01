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
#include <ll/File_ll.h>
#include "TestRoutines.h"


int main(void)
{
	/*add record to DB*/
	srand(time(NULL));

	NorDB_HWLayer *File_Hw = Filell_Init("Nor.db",4096,4);	
	NorDB_t *DB = NorDB(File_Hw, GetDummyRecordSize());

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;
	printf("NorDB Tests\n\tSectors:%d\n",Sector_Number);
	printf("\tRecordPerSector:%d\n",Record_Sector);
	printf("\tTotal Capacity:%d\n",Total_Capacity);

	printf("\n**********FILE TEST**********\n");
	RunTest(FullFill_Test, DB, 1);
	RunTest(RoundRobin_Test, DB, 1);
	RunTest(OverWrite_Test, DB, 1);
	RunTest(DeleteDB_Test, DB, 10);
	RunTest(Erase_Test, DB, 100);

	return EXIT_SUCCESS;
}
