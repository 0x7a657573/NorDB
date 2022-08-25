/*
 ============================================================================
 Name        : NorDB.c
 Author      : Mohammad Mazarei
 Version     :
 Copyright   : Copyright 2020 Mohammad Mazarei This program is free software
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NorDB.h>
#include <ll/Ram_ll.h>

typedef struct  __attribute__((__packed__))
{
	uint32_t id;
	uint8_t  dump[11];
}dummy_t;


int main(void)
{
	NorDB_HWLayer *RAM_Hw = Ramll_Init(512,2);
	NorDB_t *DB = NorDB(RAM_Hw,sizeof(dummy_t));

	dummy_t temp;
	for(int i=0;i<29;i++)
	{
		temp.id = i;
		memset(temp.dump,i,11);
		uint32_t x = NorDB_AddRecord(DB, &temp);
		printf("%i Add to %i\n",i,x);
	}

	NorDB_t *DB_New = NorDB(RAM_Hw,sizeof(dummy_t));
	printf("Unread Point is %i\n",NorDB_Get_TotalUnreadRecord(DB_New));
	for(int i=0;i<29;i++)
	{
		temp.id = i;
		memset(temp.dump,i,11);
		uint32_t x = NorDB_AddRecord(DB_New, &temp);
		printf("%i Add to %i\n",i,x);
	}
	printf("Unread Point is %i\n",NorDB_Get_TotalUnreadRecord(DB));

	for(int i=0;i<29;i++)
	{
		uint32_t x = NorDB_ReadRecord(DB_New, &temp);
		printf("%i Read from %i -> %i\n",i,x,temp.id);
	}

	for(int i=0;i<1;i++)
	{
		temp.id = i;
		memset(temp.dump,i,11);
		uint32_t x = NorDB_AddRecord(DB, &temp);
		printf("%i Add to %i\n",i,x);
	}

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
