#include "TestRoutines.h"


typedef struct  __attribute__((__packed__))
{
	uint8_t  dummy[11];
	uint32_t checksum;
}dummy_t;

    
int GetDummyRecordSize(void)
{
    return sizeof(dummy_t);
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

bool check_dummyRecord(dummy_t *rec)
{
	uint32_t sum = 0;
	for(int i=0;i<11;i++)
	{
		sum += rec->dummy[i];
	}
	return sum==rec->checksum;
}

bool FullFill_Test(NorDB_t *DB, char *name, int count)
{
	dummy_t temp;

    strcpy(name, "Full Fill");

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;

	uint32_t Used_Record = NorDB_Get_TotalUnreadRecord(DB);
	printf("--->NorDB Full Fill Test\n");

	if(Used_Record)
	{
		
		printf("\tHave %d record try empty DB\n",Used_Record);
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
			printf("\tError to add Rec %i\n",i);
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("\tSector %d Fill Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}
	printf("\tinsert %d Record in DB Completed\n\n",Total_Capacity);

	/*Try ReadBack*/
	LastUpdateSector = -1;
	for(int i=0;i<Total_Capacity;i++)
	{
		/*get random record*/
		uint32_t x = NorDB_ReadRecord(DB, &temp);
		if(x==0)
		{
			printf("\tError to Read Rec %i\n",i);
			return false;
		}

		if(check_dummyRecord(&temp)==false)
		{
			printf("\tRead Record not Correct!\n");
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("\tSector %d Read Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}
	printf("\tRead Back %d Record Correctly :)\n\n",Total_Capacity);

	return true;
}


bool RoundRobin_Test(NorDB_t *DB, char *name, int count)
{
	dummy_t temp;

    strcpy(name, "Round Robin");

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;

	uint32_t Used_Record = NorDB_Get_TotalUnreadRecord(DB);
	printf("--->Round Robin Test NorDB\n");
	

	/*be sure empty database*/
	if(Used_Record)
	{
		printf("\tHave %d record try empty DB\n",Used_Record);
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
				printf("\tError to add Rec %i\n",i);
				return false;
			}

			uint32_t useSector = (x/Sector_Size);
			if(WriteSector!=useSector)
			{
				printf("\tSector %d Fill Completed\n",useSector);
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
				printf("\tError to Read Rec %i\n",i);
				return false;
			}

			if(check_dummyRecord(&temp)==false)
			{
				printf("\tRead Record not Correct!\n");
				return false;
			}

			uint32_t useSector = (x/Sector_Size);
			if(readBack!=useSector)
			{
				printf("\tSector %d Read Completed\n",useSector);
				readBack = useSector;
			}
		}

		if(lastSectorUse==-1)
			lastSectorUse = WriteSector;
		else if(lastSectorUse == WriteSector)
		{
			printf("\tRound Robin Test Failed\n");	
			return false;
		}
		else
		{
			break;
		}
	} while (1);
	printf("\tRound Robin Correctly Work:)\n\n",Total_Capacity);

	return true;
}

bool OverWrite_Test(NorDB_t *DB, char *name, int count)
{
	dummy_t temp;

    strcpy(name, "Over Write");

	uint32_t Sector_Size   = DB->DB_ll->SectorSize;
	uint32_t Sector_Number = DB->DB_ll->SectorNumber;
	uint32_t Record_Sector = DB->Record_NumberInSector;
	uint32_t Total_Capacity = Record_Sector * Sector_Number;
	uint32_t Used_Record = NorDB_Get_TotalUnreadRecord(DB);
	printf("--->NorDB Over Write Test\n");

	/*be sure empty database*/
	if(Used_Record)
	{
		
		printf("\tHave %d record try empty DB\n",Used_Record);
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
			printf("\tError to add Rec %i\n",i);
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("\tSector %d Fill Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}
	printf("\tinsert %d Record in DB Completed\n\n",Total_Capacity);
	printf("\tAdd One Record More!\n");
	/*try add another record*/
	get_RandomRecord(&temp);
	uint32_t x = NorDB_AddRecord(DB, &temp);
	if(x!=0)
	{
		printf("\tOver Write Test Failed %i\n",x);
		return false;
	}
	printf("\tEvery Think is OK.\n\n");
	
	/*Try ReadBack*/
	LastUpdateSector = -1;
	for(int i=0;i<Total_Capacity;i++)
	{
		/*get random record*/
		uint32_t x = NorDB_ReadRecord(DB, &temp);
		if(x==0)
		{
			printf("\tError to Read Rec %i\n",i);
			return false;
		}

		if(check_dummyRecord(&temp)==false)
		{
			printf("\tRead Record not Correct!\n");
			return false;
		}

		uint32_t useSector = (x/Sector_Size);
		if(LastUpdateSector!=useSector)
		{
			printf("\tSector %d Read Completed\n",useSector);
			LastUpdateSector = useSector;
		}
	}

	printf("\tOver Write Protection Correctly Work:)\n\n",Total_Capacity);
	return true;
}

bool WriteRead_Time_Test(NorDB_t *DB, char *name, int count)
{
	dummy_t temp;

	double start, end, time_spent;

	start = GetTime();
	for(int i=0;i<count; i++)
	{
		get_RandomRecord(&temp);
		NorDB_AddRecord(DB, &temp);
	}
	end = GetTime();
    time_spent = end - start;
    printf("\tWrite %d records in: %f%cs\n\n", count, time_spent<0.1f?(time_spent*1000.0f):time_spent, time_spent<0.1f?'m':0);

	start = GetTime();
	for(int i=0;i<count; i++)
	{
		uint32_t x = NorDB_ReadRecord(DB, &temp);
		if(x==0)
		{
			printf("\tError to Read Rec %i\n",i);
			return false;
		}
		if(check_dummyRecord(&temp)==false)
		{
			printf("\tRead Record not Correct!\n");
			return false;
		}
	}
	end = GetTime();
    time_spent = end - start;
    printf("\tRead  %d records in: %f%cs\n\n", count, time_spent<0.1f?(time_spent*1000.0f):time_spent, time_spent<0.1f?'m':0);
	return true;
}

bool DeleteDB_Test(NorDB_t *DB, char *name, int count)
{
	dummy_t temp;
	
	NorDB_t* NewDB = malloc(sizeof(NorDB_t));
	memcpy(NewDB, DB, sizeof(NorDB_t));

	double start, end, time_spent;
	printf("\tCreate a copy from Database\n\n");
	start = GetTime();
	for(int i=0;i<count; i++)
	{
		get_RandomRecord(&temp);
		printf("\t(%3d)Write CS = %5d\n", i, temp.checksum);
		NorDB_AddRecord(DB, &temp);
	}
	end = GetTime();
    time_spent = end - start;
    printf("\tWrite  %d records in: %f%cs\n\n", count, time_spent<0.1f?(time_spent*1000.0f):time_spent, time_spent<0.1f?'m':0);

	memset(DB, 0, sizeof(NorDB_t));
	printf("\tDeleting Database...\n");
	printf("\tReplace copied Database\n\n");

	start = GetTime();
	for(int i=0;i<count; i++)
	{
		uint32_t x = NorDB_ReadRecord(NewDB, &temp);
		printf("\t(%3d)Read  CS = %5d\n", i, temp.checksum);
		if(x==0)
		{
			printf("\tError to Read Rec %i\n",i);
			return false;
		}
		if(check_dummyRecord(&temp)==false)
		{
			printf("\tRead Record not Correct!\n");
			return false;
		}
	}
	end = GetTime();
    time_spent = end - start;
    printf("\tRead  %d records in: %f%cs\n\n", count, time_spent<0.1f?(time_spent*1000.0f):time_spent, time_spent<0.1f?'m':0);
	free(NewDB);
	return true;
}

int RunTest(bool(*test)(NorDB_t*, char*, int), NorDB_t* DB, int count)
{
    char testName[20] = {0};
    srand(time(NULL));
    double start, end;

    start = GetTime();
    if(!test(DB, testName, count))
	{
		printf("%s Test Not Complete\n", testName);
		return EXIT_FAILURE;
	}
    end = GetTime();
    double time_spent = end - start;
    printf("Test Execution Time: %f%cs\n\n", time_spent<0.1f?(time_spent*1000.0f):time_spent, time_spent<0.1f?'m':0);
    return 0;
}




