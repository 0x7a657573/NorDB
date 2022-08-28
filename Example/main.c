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
#include <ll/File_ll.h>
typedef struct  __attribute__((__packed__))
{
	uint8_t  dummy[11];
	uint32_t checksum;
}dummy_t;

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

int main(void)
{
	/*add record to DB*/
	dummy_t temp;
	srand(time(NULL));

	NorDB_HWLayer *File_Hw = Filell_Init("/home/zeus/w/NorDB/build/Nor.db",512,2);	
	NorDB_t *DB = NorDB(File_Hw,sizeof(dummy_t));

	uint32_t Unread = NorDB_Get_TotalUnreadRecord(DB);
	printf("Unread Point is %i\n",Unread);
	printf("Read %i of record ...\n",Unread/2);
	for(int i=0;i<Unread/2;i++)
	{
		uint32_t x = NorDB_ReadRecord(DB, &temp);
		if(x==0)
		{
			printf("Error Read Reocrd\n");
			return 1;
		}

		if(check_bummyRecord(&temp)==false)
		{
			printf("Read Reocrd not Correct!\n");
			return 1;
		}
	}
	printf("Unread Point is %i and %d is free\n",NorDB_Get_TotalUnreadRecord(DB),NorDB_Get_FreeRecord(DB));
		
	/*mount another DB to This io layer*/
	NorDB_t *DB_New = NorDB(File_Hw,sizeof(dummy_t));
	printf("Unread Point is %i\n",NorDB_Get_TotalUnreadRecord(DB_New));
	printf("Try insert %d another Record\n",NorDB_Get_FreeRecord(DB)/3);
	for(int i=0;i<NorDB_Get_FreeRecord(DB)/3;i++)
	{
		get_RandomRecord(&temp);
		uint32_t x = NorDB_AddRecord(DB_New, &temp);
		if(x==0)
		{
			printf("Error to add Rec %i\n",i);
			return 1;
		}
	}
	uint32_t UNreadRecord = NorDB_Get_TotalUnreadRecord(DB);
	printf("Total Unread Point is %i\n",UNreadRecord/2);


	for(int i=0;i<UNreadRecord/2;i++)
	{
		uint32_t x = NorDB_ReadRecord(DB_New, &temp);
		if(x==0)
		{
			printf("Error Read Reocrd\n");
			return 1;
		}

		if(check_bummyRecord(&temp)==false)
		{
			printf("Read Reocrd not Correct!\n");
			return 1;
		}
	}
	printf("We Read Back %d Record Correctly :)\n",UNreadRecord);


	Filell_Del(File_Hw);
	return EXIT_SUCCESS;
}
