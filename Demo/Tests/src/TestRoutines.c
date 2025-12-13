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
		uint32_t w = NorDB_AddRecord(DB, &temp);
		if (w == 0)
		{
			printf("\tError to Write Rec %i (DB full or write failed)\n", i);
			return false;
		}
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

	strcpy(name, "Recovery");

	printf("--->NorDB Recovery (Restart) Test\n");

	/* start from a known state */
	NorDB_Clear(DB);

	double start, end, time_spent;
	start = GetTime();
	for (int i = 0; i < count; i++)
	{
		get_RandomRecord(&temp);
		uint32_t w = NorDB_AddRecord(DB, &temp);
		if (w == 0)
		{
			printf("\tError to Write Rec %i\n", i);
			return false;
		}
	}
	end = GetTime();
	time_spent = end - start;
	printf("\tWrite  %d records in: %f%cs\n\n", count, time_spent < 0.1f ? (time_spent * 1000.0f) : time_spent, time_spent < 0.1f ? 'm' : 0);

	/* simulate reboot: force re-sync from storage */
	NorDB_HWLayer *hw = DB->DB_ll;
	hw->Synced = false;

	NorDB_t *NewDB = NorDB(hw, GetDummyRecordSize());
	if (!NewDB)
	{
		printf("\tError: NorDB re-init failed\n");
		return false;
	}

	uint32_t unread_after_reboot = NorDB_Get_TotalUnreadRecord(NewDB);
	if (unread_after_reboot != (uint32_t)count)
	{
		printf("\tError: Unread count mismatch after reboot. expected=%d got=%d\n", count, unread_after_reboot);
		NorDB_Destroy(NewDB);
		return false;
	}

	start = GetTime();
	for (int i = 0; i < count; i++)
	{
		uint32_t x = NorDB_ReadRecord(NewDB, &temp);
		if (x == 0)
		{
			printf("\tError to Read Rec %i\n", i);
			NorDB_Destroy(NewDB);
			return false;
		}
		if (check_dummyRecord(&temp) == false)
		{
			printf("\tRead Record not Correct!\n");
			NorDB_Destroy(NewDB);
			return false;
		}
	}
	end = GetTime();
	time_spent = end - start;
	printf("\tRead  %d records in: %f%cs\n\n", count, time_spent < 0.1f ? (time_spent * 1000.0f) : time_spent, time_spent < 0.1f ? 'm' : 0);

	NorDB_Destroy(NewDB);
	return true;
}

bool Clear_Test(NorDB_t *DB, char *name, int count)
{
    dummy_t temp;
    strcpy(name, "Clear");

    printf("--->NorDB Clear Test\n");

    // 1. Fill the database with some records
    printf("\tAdding %d records to the database...\n", count);
    for (int i = 0; i < count; i++)
    {
        get_RandomRecord(&temp);
        if (NorDB_AddRecord(DB, &temp) == 0)
        {
            printf("\tError adding record %d\n", i);
            return false;
        }
    }

    uint32_t records_before_erase = NorDB_Get_TotalUnreadRecord(DB);
    printf("\tRecords in DB before erase: %d\n", records_before_erase);

    if (records_before_erase != count)
    {
        printf("\tError: Record count mismatch before erase.\n");
        return false;
    }

    // 2. Erase the database
    printf("\tErasing the database...\n");
    if (!NorDB_Clear(DB))
    {
        printf("\tError erasing the database.\n");
        return false;
    }
    printf("\tErase operation completed.\n");

    // 3. Verify that the database is empty
    uint32_t records_after_erase = NorDB_Get_TotalUnreadRecord(DB);
    printf("\tRecords in DB after erase: %d\n", records_after_erase);

    if (records_after_erase != 0)
    {
        printf("\tError: Database is not empty after erase.\n");
        return false;
    }

    // 4. Verify that the database is still usable
    printf("\tAdding a new record to verify usability...\n");
    get_RandomRecord(&temp);
    if (NorDB_AddRecord(DB, &temp) == 0)
    {
        printf("\tError adding a record after erase.\n");
        return false;
    }

    uint32_t records_after_add = NorDB_Get_TotalUnreadRecord(DB);
    printf("\tRecords in DB after adding one more record: %d\n", records_after_add);

    if (records_after_add != 1)
    {
        printf("\tError: Could not add a new record after erase.\n");
        return false;
    }
    
    printf("\tErase test completed successfully :)\n\n");
    return true;
}

bool ReadEmpty_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	dummy_t temp;
	strcpy(name, "Read Empty");

	printf("--->NorDB Read Empty Test\n");

	NorDB_Clear(DB);

	uint32_t unread = NorDB_Get_TotalUnreadRecord(DB);
	if (unread != 0)
	{
		printf("\tError: DB is not empty after clear. unread=%d\n", unread);
		return false;
	}

	uint32_t x = NorDB_ReadRecord(DB, &temp);
	if (x != 0)
	{
		printf("\tError: expected ReadRecord=0 on empty DB, got=%d\n", x);
		return false;
	}

	printf("\tRead from empty DB correctly returned 0\n\n");
	return true;
}

bool CRC_DetectsCorruption_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	dummy_t r1, r2, out;
	strcpy(name, "CRC Corruption");

	printf("--->NorDB CRC Detects Corruption Test\n");

	NorDB_Clear(DB);

	get_RandomRecord(&r1);
	get_RandomRecord(&r2);

	uint32_t adr1 = NorDB_AddRecord(DB, &r1);
	uint32_t adr2 = NorDB_AddRecord(DB, &r2);
	if (adr1 == 0 || adr2 == 0)
	{
		printf("\tError: failed to add records (adr1=%u adr2=%u)\n", adr1, adr2);
		return false;
	}

	/* corrupt first record payload in-place (keep stored CRC byte unchanged) */
	uint8_t b = 0;
	DB->DB_ll->ReadBuffer(DB->DB_ll->Param, adr1, &b, 1);
	b ^= 0xFF;
	DB->DB_ll->WriteBuffer(DB->DB_ll->Param, adr1, &b, 1);

	uint32_t x = NorDB_ReadRecord(DB, &out);
	if (x != 0)
	{
		printf("\tError: expected ReadRecord=0 on corrupted record, got adr=%u\n", x);
		return false;
	}
	printf("\tCorrupted record correctly rejected (ReadRecord returned 0)\n");

	/* next record should still be readable and correct */
	x = NorDB_ReadRecord(DB, &out);
	if (x == 0)
	{
		printf("\tError: failed to read next record after corruption\n");
		return false;
	}
	if (check_dummyRecord(&out) == false)
	{
		printf("\tError: next record data invalid after corruption\n");
		return false;
	}

	printf("\tNext record still readable and valid\n\n");
	return true;
}


int RunTest(bool(*test)(NorDB_t*, char*, int), NorDB_t* DB, int count)
{
    char testName[20] = {0};
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
