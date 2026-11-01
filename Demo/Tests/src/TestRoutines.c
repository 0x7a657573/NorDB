#include "TestRoutines.h"
#include <stddef.h>


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

    // start empty and never ask for more than the DB can hold
    NorDB_Clear(DB);
    uint32_t writable = NorDB_Get_FreeRecord(DB);
    if (count > (int)writable)
        count = writable;

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

	uint32_t crc_before = NorDB_Get_CrcErrorCount(DB);

	/* corrupt record is skipped; the next valid record is returned */
	uint32_t x = NorDB_ReadRecord(DB, &out);
	if (x != adr2)
	{
		printf("\tError: expected ReadRecord=%u (next valid record), got adr=%u\n", adr2, x);
		return false;
	}
	if (memcmp(&out, &r2, sizeof(out)) != 0 || check_dummyRecord(&out) == false)
	{
		printf("\tError: next record data invalid after corruption\n");
		return false;
	}
	if (NorDB_Get_CrcErrorCount(DB) != crc_before + 1)
	{
		printf("\tError: CrcErrorCount expected %u, got %u\n", crc_before + 1, NorDB_Get_CrcErrorCount(DB));
		return false;
	}
	printf("\tCorrupted record skipped, CrcErrorCount incremented\n");

	if (NorDB_ReadRecord(DB, &out) != 0)
	{
		printf("\tError: expected empty DB after reading all valid records\n");
		return false;
	}

	printf("\tNo valid record left after skipping corruption\n\n");
	return true;
}


bool PowerLoss_PhantomRecord_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	dummy_t r[3], r4, out;
	strcpy(name, "PowerLoss Phantom");

	printf("--->NorDB Power Loss Phantom Record Test\n");

	NorDB_HWLayer *hw = DB->DB_ll;
	uint32_t adr = 0;
	uint32_t idx = 0;

	NorDB_Clear(DB);

	/* the phantom slot must be in the same sector as the last record */
	for (int attempt = 0; attempt < 8; attempt++)
	{
		for (int i = 0; i < 3; i++)
		{
			get_RandomRecord(&r[i]);
			adr = NorDB_AddRecord(DB, &r[i]);
			if (adr == 0)
			{
				printf("\tError: failed to add record %d\n", i);
				return false;
			}
		}
		idx = ((adr % hw->SectorSize) - DB->Header_Size) / DB->Record_Size;
		if (idx + 1 < DB->Record_NumberInSector)
			break;
		/* no room after this record: consume and retry in the next sector */
		for (int i = 0; i < 3; i++)
			NorDB_ReadRecord(DB, &out);
		idx = DB->Record_NumberInSector;
	}
	if (idx + 1 >= DB->Record_NumberInSector)
	{
		printf("\tError: could not place records before the end of a sector\n");
		return false;
	}

	/* power loss: next slot marked unread, payload never written */
	idx++;
	uint32_t byte_adr = (adr / hw->SectorSize) * hw->SectorSize + sizeof(NorDB_Header_t) + (idx / 4);
	int bit_shift = (idx % 4) * 2;
	uint8_t b = 0;
	hw->ReadBuffer(hw->Param, byte_adr, &b, 1);
	b &= ~(nordb_FreeMark << bit_shift);
	b |= nordb_UnReadMark << bit_shift;
	hw->WriteBuffer(hw->Param, byte_adr, &b, 1);

	/* simulate reboot: a second NorDB_t on the same hw runs the sync */
	uint32_t crc_before = NorDB_Get_CrcErrorCount(DB);
	hw->Synced = false;
	NorDB_t *DB2 = NorDB(hw, GetDummyRecordSize());
	if (DB2 == NULL)
	{
		printf("\tError: failed to re-create DB\n");
		return false;
	}

	bool ok = false;
	if (NorDB_Get_TotalUnreadRecord(DB2) != 3)
	{
		printf("\tError: expected 3 unread after sync, got %u\n", NorDB_Get_TotalUnreadRecord(DB2));
		goto done;
	}
	if (NorDB_Get_CrcErrorCount(DB2) != crc_before + 1)
	{
		printf("\tError: CrcErrorCount expected %u, got %u\n", crc_before + 1, NorDB_Get_CrcErrorCount(DB2));
		goto done;
	}
	for (int i = 0; i < 3; i++)
	{
		if (NorDB_ReadRecord(DB2, &out) == 0 || memcmp(&out, &r[i], sizeof(out)) != 0)
		{
			printf("\tError: record %d not valid after sync\n", i);
			goto done;
		}
	}
	if (NorDB_ReadRecord(DB2, &out) != 0 || NorDB_Get_CrcErrorCount(DB2) != crc_before + 1)
	{
		printf("\tError: phantom record still readable\n");
		goto done;
	}
	printf("\tPhantom record dropped at sync, 3 valid records intact\n");

	get_RandomRecord(&r4);
	if (NorDB_AddRecord(DB2, &r4) == 0 || NorDB_ReadRecord(DB2, &out) == 0 ||
		memcmp(&out, &r4, sizeof(out)) != 0)
	{
		printf("\tError: DB not usable after repair\n");
		goto done;
	}
	printf("\tDB still accepts records after repair\n\n");
	ok = true;

done:
	NorDB_Destroy(DB2);
	return ok;
}


static uint32_t FreeRecord_FillUntilFail(NorDB_t *DB)
{
	dummy_t r;
	uint32_t n = 0;
	get_RandomRecord(&r);
	while (NorDB_AddRecord(DB, &r) != 0)
		n++;
	return n;
}

static bool FreeRecord_Check(NorDB_t *DB, const char *pattern)
{
	uint32_t free_rec = NorDB_Get_FreeRecord(DB);
	uint32_t added = FreeRecord_FillUntilFail(DB);
	if (free_rec != added)
	{
		printf("\tError (%s): FreeRecord=%u but AddRecord accepted %u\n", pattern, free_rec, added);
		return false;
	}
	printf("\t%s: FreeRecord=%u matches\n", pattern, free_rec);
	return true;
}

static uint32_t FreeRecord_Add(NorDB_t *DB, uint32_t n)
{
	dummy_t r;
	uint32_t done = 0;
	get_RandomRecord(&r);
	while (done < n && NorDB_AddRecord(DB, &r) != 0)
		done++;
	return done;
}

static void FreeRecord_Read(NorDB_t *DB, uint32_t n)
{
	dummy_t r;
	for (uint32_t i = 0; i < n; i++)
		NorDB_ReadRecord(DB, &r);
}

/* read up to and including the last slot of the sector the read head is in */
static void FreeRecord_ReadSector(NorDB_t *DB)
{
	dummy_t r;
	uint32_t adr;
	while ((adr = NorDB_ReadRecord(DB, &r)) != 0)
	{
		uint32_t idx = ((adr % DB->DB_ll->SectorSize) - DB->Header_Size) / DB->Record_Size;
		if (idx + 1 >= DB->Record_NumberInSector)
			break;
	}
}

/* empty DB whose last written sector is completely used, so patterns start sector aligned */
static void FreeRecord_Prepare(NorDB_t *DB)
{
	dummy_t r;
	uint32_t adr;
	NorDB_Clear(DB);
	get_RandomRecord(&r);
	while ((adr = NorDB_AddRecord(DB, &r)) != 0)
	{
		uint32_t idx = ((adr % DB->DB_ll->SectorSize) - DB->Header_Size) / DB->Record_Size;
		if (idx + 1 >= DB->Record_NumberInSector)
			break;
	}
	NorDB_Clear(DB);
}

bool FreeRecord_Accuracy_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	strcpy(name, "FreeRecord Accuracy");

	printf("--->NorDB FreeRecord Accuracy Test\n");

	uint32_t per_sector = DB->Record_NumberInSector;
	bool ok = true;

	NorDB_Clear(DB);
	ok = ok && FreeRecord_Check(DB, "empty");

	FreeRecord_Prepare(DB);
	ok = ok && FreeRecord_Check(DB, "empty, aligned");

	FreeRecord_Prepare(DB);
	FreeRecord_Add(DB, 3);
	ok = ok && FreeRecord_Check(DB, "partial sector");

	FreeRecord_Prepare(DB);
	FreeRecord_Add(DB, UINT32_MAX);
	ok = ok && FreeRecord_Check(DB, "full");

	FreeRecord_Prepare(DB);
	FreeRecord_Add(DB, UINT32_MAX);
	FreeRecord_Read(DB, 7);
	ok = ok && FreeRecord_Check(DB, "full, 7 read");

	FreeRecord_Prepare(DB);
	FreeRecord_Add(DB, UINT32_MAX);
	FreeRecord_Read(DB, per_sector / 2);
	ok = ok && FreeRecord_Check(DB, "half sector read");

	FreeRecord_Prepare(DB);
	FreeRecord_Add(DB, UINT32_MAX);
	FreeRecord_ReadSector(DB);
	ok = ok && FreeRecord_Check(DB, "one sector read");

	FreeRecord_Prepare(DB);
	FreeRecord_Add(DB, UINT32_MAX);
	FreeRecord_ReadSector(DB);
	FreeRecord_Read(DB, 3);
	FreeRecord_Add(DB, 4);
	ok = ok && FreeRecord_Check(DB, "wrap-around");

	/* leave the DB empty for the following tests */
	NorDB_Clear(DB);
	if (!ok)
		return false;

	printf("\n");
	return true;
}

bool IncompatibleHeader_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	dummy_t r[3], out;
	strcpy(name, "Incompatible Header");

	printf("--->NorDB Incompatible Header Test\n");

	NorDB_HWLayer *hw = DB->DB_ll;
	uint16_t size = GetDummyRecordSize();

	NorDB_Clear(DB);
	for (int i = 0; i < 3; i++)
	{
		get_RandomRecord(&r[i]);
		if (NorDB_AddRecord(DB, &r[i]) == 0)
		{
			printf("\tError: failed to add record %d\n", i);
			return false;
		}
	}

	/* (1) storage in use with another size */
	if (NorDB(hw, size + 1) != NULL)
	{
		printf("\tError: NorDB() accepted another record size on synced storage\n");
		return false;
	}
	printf("\tOther record size rejected on synced storage\n");

	/* (2) not synced: the scan must refuse and keep the data */
	hw->Synced = false;
	if (NorDB(hw, size + 1) != NULL)
	{
		printf("\tError: NorDB() accepted storage written with another record size\n");
		return false;
	}
	hw->Synced = true;
	if (NorDB_Get_TotalUnreadRecord(DB) != 3)
	{
		printf("\tError: unread count changed to %u\n", NorDB_Get_TotalUnreadRecord(DB));
		return false;
	}
	for (int i = 0; i < 3; i++)
	{
		if (NorDB_ReadRecord(DB, &out) == 0 || memcmp(&out, &r[i], sizeof(out)) != 0)
		{
			printf("\tError: record %d lost after rejected NorDB()\n", i);
			return false;
		}
	}
	printf("\tOther record size rejected on stored data, data intact\n");

	/* (3) deliberate format */
	if (!NorDB_Format(hw))
	{
		printf("\tError: NorDB_Format failed\n");
		return false;
	}
	NorDB_t *DB2 = NorDB(hw, size);
	if (DB2 == NULL)
	{
		printf("\tError: failed to re-create DB after format\n");
		return false;
	}
	bool ok = false;
	if (NorDB_Get_TotalUnreadRecord(DB2) != 0)
	{
		printf("\tError: DB not empty after format (%u)\n", NorDB_Get_TotalUnreadRecord(DB2));
		goto done;
	}
	get_RandomRecord(&r[0]);
	if (NorDB_AddRecord(DB2, &r[0]) == 0 || NorDB_ReadRecord(DB2, &out) == 0 ||
		memcmp(&out, &r[0], sizeof(out)) != 0)
	{
		printf("\tError: DB not usable after format\n");
		goto done;
	}
	ok = true;

done:
	NorDB_Destroy(DB2);
	if (!ok)
		return false;

	/* the original handle must still work */
	get_RandomRecord(&r[1]);
	if (NorDB_AddRecord(DB, &r[1]) == 0 || NorDB_ReadRecord(DB, &out) == 0 ||
		memcmp(&out, &r[1], sizeof(out)) != 0)
	{
		printf("\tError: original DB not usable after format\n");
		return false;
	}
	NorDB_Clear(DB);
	printf("\tFormat recovered the storage\n\n");
	return true;
}

static void Fifo_MakeRecord(dummy_t *r, uint32_t seq)
{
	r->checksum = 0;
	for (int i = 0; i < 11; i++)
	{
		r->dummy[i] = (i < 4) ? (uint8_t)(seq >> (8 * i)) : (uint8_t)(seq * 7 + i);
		r->checksum += r->dummy[i];
	}
}

/* write up to n records (stops when the DB is full), returns how many were written */
static uint32_t Fifo_Write(NorDB_t *DB, uint32_t *next_w, uint32_t n)
{
	dummy_t r;
	uint32_t done = 0;
	while (done < n)
	{
		Fifo_MakeRecord(&r, *next_w);
		if (NorDB_AddRecord(DB, &r) == 0)
			break;
		(*next_w)++;
		done++;
	}
	return done;
}

/* read up to n records, each must carry the next expected sequence number */
static bool Fifo_Read(NorDB_t *DB, uint32_t *next_r, uint32_t n, uint32_t *got)
{
	dummy_t r;
	*got = 0;
	while (*got < n && NorDB_ReadRecord(DB, &r) != 0)
	{
		uint32_t seq = r.dummy[0] | (r.dummy[1] << 8) | (r.dummy[2] << 16) | ((uint32_t)r.dummy[3] << 24);
		if (check_dummyRecord(&r) == false || seq != *next_r)
		{
			printf("\tError: read sequence %u, expected %u\n", seq, *next_r);
			return false;
		}
		(*next_r)++;
		(*got)++;
	}
	return true;
}

bool FifoOrder_Wrap_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	strcpy(name, "FIFO Order Wrap");

	printf("--->NorDB FIFO Order Wrap Test\n");

	uint32_t per_sector = DB->Record_NumberInSector;
	uint32_t sectors = DB->DB_ll->SectorNumber;
	uint32_t w = 0, r = 0, got = 0, n = 0;

	NorDB_Clear(DB);

	/* drain, then wrap several full laps one sector at a time */
	for (uint32_t i = 0; i < 3 * sectors; i++)
	{
		n = Fifo_Write(DB, &w, per_sector);
		if (n != per_sector || !Fifo_Read(DB, &r, per_sector, &got) || got != per_sector)
		{
			printf("\tError: lap %u failed (written %u read %u)\n", i, n, got);
			return false;
		}
	}
	printf("\tDrain-then-wrap laps in order\n");

	/* drain completely, then write across the wrap point */
	for (uint32_t i = 0; i < sectors + 1; i++)
	{
		n = Fifo_Write(DB, &w, (sectors - 1) * per_sector + 1);
		if (n != (sectors - 1) * per_sector + 1 || !Fifo_Read(DB, &r, UINT32_MAX, &got) || got != n)
		{
			printf("\tError: wrap burst %u failed (written %u read %u)\n", i, n, got);
			return false;
		}
		/* start the next burst from a different offset inside the sector */
		Fifo_Write(DB, &w, i + 1);
		if (!Fifo_Read(DB, &r, i + 1, &got) || got != i + 1)
		{
			printf("\tError: offset step %u failed\n", i);
			return false;
		}
	}
	printf("\tWrite across wrap point in order\n");

	/* partial drains: refill whatever the writer accepts, read part of it */
	for (uint32_t i = 0; i < 4 * sectors; i++)
	{
		Fifo_Write(DB, &w, UINT32_MAX);
		if (!Fifo_Read(DB, &r, (i % 3 + 1) * per_sector / 2 + 1, &got))
			return false;
	}
	if (!Fifo_Read(DB, &r, UINT32_MAX, &got))
		return false;
	if (r != w)
	{
		printf("\tError: wrote %u records but read %u\n", w, r);
		return false;
	}
	printf("\tPartial drains in order, %u records written and read once\n\n", w);

	/* leave the DB empty for the following tests */
	NorDB_Clear(DB);
	return true;
}

bool PowerLoss_SectorHeader_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	strcpy(name, "PowerLoss Header");

	printf("--->NorDB Power Loss Sector Header Test\n");

	NorDB_HWLayer *hw = DB->DB_ll;
	uint32_t per_sector = DB->Record_NumberInSector;
	uint32_t w = 0, r = 0, got = 0, n = 0;

	/* sector aligned start: the last written sector is full, everything is read */
	FreeRecord_Prepare(DB);

	/* one full sector of unread records, the writer is about to open the next sector */
	n = Fifo_Write(DB, &w, per_sector);
	if (n != per_sector)
	{
		printf("\tError: wrote %u of %u records\n", n, per_sector);
		return false;
	}

	/* interrupted NorDB_Init_Sector: sector erased, header body programmed, Magic not yet */
	uint32_t next = (hw->LastWriteSector + 1) % hw->SectorNumber;
	NorDB_Header_t body;
	body.Vertion = NorDB_RVer;
	body.SyncCounter = hw->SyncCounter + 1;
	body.RecordSize = DB->Record_Size;
	hw->SectorErace(hw->Param, next * hw->SectorSize);
	hw->WriteBuffer(hw->Param, next * hw->SectorSize + offsetof(NorDB_Header_t, Vertion),
			(uint8_t*)&body + offsetof(NorDB_Header_t, Vertion),
			offsetof(NorDB_Header_t, Records) - offsetof(NorDB_Header_t, Vertion));

	/* reboot */
	hw->Synced = false;
	NorDB_t *DB2 = NorDB(hw, GetDummyRecordSize());
	if (DB2 == NULL)
	{
		printf("\tError: DB refused to open after interrupted header write\n");
		return false;
	}

	bool ok = false;
	if (NorDB_Get_TotalUnreadRecord(DB2) != per_sector)
	{
		printf("\tError: expected %u unread, got %u\n", per_sector, NorDB_Get_TotalUnreadRecord(DB2));
		goto done;
	}
	if (!Fifo_Read(DB2, &r, UINT32_MAX, &got) || got != per_sector)
	{
		printf("\tError: read %u of %u records back\n", got, per_sector);
		goto done;
	}
	printf("\tDB opens, records intact and in order\n");

	/* keep writing across the half-initialized sector, two full laps */
	for (int lap = 0; lap < 2; lap++)
	{
		n = Fifo_Write(DB2, &w, UINT32_MAX);
		if (n < per_sector || !Fifo_Read(DB2, &r, UINT32_MAX, &got) || got != n)
		{
			printf("\tError: lap %d failed (written %u read %u)\n", lap, n, got);
			goto done;
		}
	}
	if (r != w)
	{
		printf("\tError: wrote %u records but read %u\n", w, r);
		goto done;
	}
	printf("\tWriting continues across the interrupted sector in order\n\n");
	ok = true;

done:
	NorDB_Destroy(DB2);
	NorDB_Clear(DB);
	return ok;
}

bool InitParams_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	strcpy(name, "Init Params");

	printf("--->NorDB Init Parameters Test\n");

	NorDB_HWLayer *hw = DB->DB_ll;

	if (NorDB(hw, 0) != NULL || NorDB(hw, 0xFFFF) != NULL || NorDB(hw, 0xFFFE) != NULL ||
		NorDB(hw, hw->SectorSize) != NULL || NorDB(NULL, GetDummyRecordSize()) != NULL)
	{
		printf("\tError: NorDB() accepted invalid parameters\n");
		return false;
	}
	printf("\tInvalid record sizes rejected\n\n");
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
