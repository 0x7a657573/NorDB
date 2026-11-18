#include "TestRoutines.h"
#include <stddef.h>

static int tests_run;
static int tests_failed;

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

	printf("--->NorDB Full Fill Test\n");

	if(!NorDB_Format(DB->DB_ll))
	{
		printf("\tError: NorDB_Format failed\n");
		return false;
	}

	if(NorDB_Get_TotalUnreadRecord(DB) != 0)
	{
		printf("\tError: DB not empty after format (%u)\n",NorDB_Get_TotalUnreadRecord(DB));
		return false;
	}

	if(NorDB_Get_FreeRecord(DB) != Total_Capacity)
	{
		printf("\tError: expected %u free records after format, got %u\n",
			Total_Capacity, NorDB_Get_FreeRecord(DB));
		return false;
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


/* power loss: mark slot idx of a sector unread without writing its payload */
static void MarkSlotUnread(NorDB_HWLayer *hw, uint32_t sector_adr, uint32_t idx)
{
	uint32_t byte_adr = sector_adr + sizeof(NorDB_Header_t) + (idx / 4);
	int bit_shift = (idx % 4) * 2;
	uint8_t b = 0;
	hw->ReadBuffer(hw->Param, byte_adr, &b, 1);
	b &= ~(nordb_FreeMark << bit_shift);
	b |= nordb_UnReadMark << bit_shift;
	hw->WriteBuffer(hw->Param, byte_adr, &b, 1);
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
	MarkSlotUnread(hw, (adr / hw->SectorSize) * hw->SectorSize, idx);

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


#define ERASED_CRC_MAX_PAYLOAD 254

/* fresh storage; with two or more slots per sector one valid record, then the next slot of
   that sector, else (single slot) slot 0 of sector 0 where the first write would go; the
   phantom slot is marked unread with `partial` bytes of payload written (0 = erased slot)
   and its crc byte stays erased */
static bool ErasedCrc_Case(NorDB_HWLayer *hw, uint16_t size, const uint8_t *partial,
						   uint16_t partial_len, const char *what)
{
	uint8_t w[ERASED_CRC_MAX_PAYLOAD], out[ERASED_CRC_MAX_PAYLOAD];
	bool ok = false;

	if (!NorDB_Format(hw))
		return false;
	NorDB_t *DB = NorDB(hw, size);
	if (DB == NULL)
	{
		printf("\tError: NorDB(%u) failed\n", size);
		return false;
	}
	bool has_valid = DB->Record_NumberInSector >= 2;
	uint32_t sector_adr = 0;
	uint32_t idx = 0;
	for (uint16_t i = 0; i < size; i++)
		w[i] = (uint8_t)(rand() & 0xFF);
	if (has_valid)
	{
		uint32_t adr = NorDB_AddRecord(DB, w);
		if (adr == 0)
		{
			printf("\tError: failed to add record (size %u)\n", size);
			goto done;
		}
		sector_adr = (adr / hw->SectorSize) * hw->SectorSize;
		idx = ((adr % hw->SectorSize) - DB->Header_Size) / DB->Record_Size + 1;
		if (idx >= DB->Record_NumberInSector)
		{
			printf("\tError: no free slot after the record (size %u)\n", size);
			goto done;
		}
	}
	else
	{
		/* no record to initialize sector 0, write its header as the first write would */
		NorDB_Header_t hdr = { NorDB_Magic, NorDB_RVer, 1, DB->Record_Size };
		if (!hw->WriteBuffer(hw->Param, 0, (uint8_t*)&hdr, sizeof(hdr)))
		{
			printf("\tError: failed to write sector header (size %u)\n", size);
			goto done;
		}
	}
	uint32_t slot_adr = sector_adr + DB->Header_Size + idx * DB->Record_Size;
	MarkSlotUnread(hw, sector_adr, idx);
	if (partial_len)
		hw->WriteBuffer(hw->Param, slot_adr, (uint8_t*)partial, partial_len);

	/* simulate reboot */
	uint32_t crc_before = NorDB_Get_CrcErrorCount(DB);
	NorDB_Destroy(DB);
	DB = NULL;
	hw->Synced = false;
	DB = NorDB(hw, size);
	if (DB == NULL)
	{
		printf("\tError: failed to re-create DB (size %u)\n", size);
		return false;
	}
	if (NorDB_Get_TotalUnreadRecord(DB) != has_valid || NorDB_Get_CrcErrorCount(DB) != crc_before + 1)
	{
		printf("\tError: %s size %u: unread=%u (exp %u) crc_errors=%u (exp %u)\n", what, size,
			   NorDB_Get_TotalUnreadRecord(DB), has_valid, NorDB_Get_CrcErrorCount(DB), crc_before + 1);
		goto done;
	}
	if (has_valid && (NorDB_ReadRecord(DB, out) == 0 || memcmp(out, w, size) != 0))
	{
		printf("\tError: %s size %u: valid record lost\n", what, size);
		goto done;
	}
	if (NorDB_ReadRecord(DB, out) != 0)
	{
		printf("\tError: %s size %u: fake record delivered\n", what, size);
		goto done;
	}
	/* the repaired DB must keep working */
	for (uint16_t i = 0; i < size; i++)
		w[i] = (uint8_t)(rand() & 0xFF);
	if (NorDB_AddRecord(DB, w) == 0 || NorDB_ReadRecord(DB, out) == 0 || memcmp(out, w, size) != 0)
	{
		printf("\tError: %s size %u: DB not usable after repair\n", what, size);
		goto done;
	}
	printf("\t%s dropped at sync (size %u, %s), DB still usable\n", what, size,
		   has_valid ? "valid record kept" : "single slot");
	ok = true;

done:
	NorDB_Destroy(DB);
	return ok;
}

bool ErasedCrc_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	uint8_t w[ERASED_CRC_MAX_PAYLOAD], out[ERASED_CRC_MAX_PAYLOAD];
	NorDB_HWLayer *hw = DB->DB_ll;
	bool ok = false;
	strcpy(name, "Erased CRC");

	printf("--->NorDB Erased CRC Test\n");

	/* payload sizes where the crc8 of an all 0xFF payload is 0xFF */
	for (uint16_t size = 127; size <= ERASED_CRC_MAX_PAYLOAD; size += 127)
	{
		if (size + 1u + sizeof(NorDB_Header_t) + 1u > hw->SectorSize)
		{
			printf("\tRecord size %u does not fit in a sector, skipped\n", size);
			continue;
		}
		/* a partial payload whose crc is 0xFF, followed by an erased crc byte */
		memset(w, 0xFF, size);
		w[0] = 0;
		int found = 0;
		for (int b = 0; b < 256 && !found; b++)
		{
			w[1] = (uint8_t)b;
			found = NorDB_crc8x_fast(w, size) == 0xFF;
		}
		if (!found)
		{
			printf("\tError: no partial payload with crc 0xFF (size %u)\n", size);
			goto done;
		}

		/* (1) slot marked unread, still fully erased */
		if (!ErasedCrc_Case(hw, size, NULL, 0, "Erased slot"))
			goto done;
		/* (3) partial payload, erased crc byte */
		if (!ErasedCrc_Case(hw, size, w, size - 1, "Partial payload"))
			goto done;

		/* (2) a real all 0xFF record is a valid record */
		if (!NorDB_Format(hw))
			goto done;
		NorDB_t *DB2 = NorDB(hw, size);
		if (DB2 == NULL)
		{
			printf("\tError: NorDB(%u) failed\n", size);
			goto done;
		}
		memset(w, 0xFF, size);
		memset(out, 0, size);
		bool rec_ok = NorDB_AddRecord(DB2, w) != 0 && NorDB_ReadRecord(DB2, out) != 0 &&
					  memcmp(out, w, size) == 0 && NorDB_Get_TotalUnreadRecord(DB2) == 0;
		NorDB_Destroy(DB2);
		if (!rec_ok)
		{
			printf("\tError: all 0xFF record not delivered intact (size %u)\n", size);
			goto done;
		}
		printf("\tAll 0xFF record delivered intact (size %u)\n", size);
	}
	printf("\n");
	ok = true;

done:
	/* give the storage back to the record size of the other tests */
	if (!NorDB_Format(hw))
		return false;
	NorDB_t *DB3 = NorDB(hw, GetDummyRecordSize());
	if (DB3 == NULL)
		return false;
	NorDB_Destroy(DB3);
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

bool PowerLoss_Erase_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	strcpy(name, "PowerLoss Erase");

	printf("--->NorDB Power Loss Sector Erase Test\n");

	NorDB_HWLayer *hw = DB->DB_ll;
	uint32_t per_sector = DB->Record_NumberInSector;
	uint32_t w = 0, r = 0, got = 0, n = 0;

	/* sector aligned start: the last written sector is full, everything is read */
	FreeRecord_Prepare(DB);
	uint32_t old = hw->LastWriteSector;

	/* the next sector holds one full sector of unread records */
	r = w;
	n = Fifo_Write(DB, &w, per_sector);
	if (n != per_sector || hw->LastWriteSector == old)
	{
		printf("\tError: wrote %u of %u records\n", n, per_sector);
		return false;
	}

	/* interrupted reuse of the consumed sector: Magic already zeroed, erase cut short */
	uint16_t dead = 0;
	hw->WriteBuffer(hw->Param, old * hw->SectorSize, (uint8_t*)&dead, sizeof(dead));
	uint32_t seed = 12345;
	for (uint32_t off = sizeof(dead); off < hw->SectorSize; off += 32)
	{
		uint8_t buf[32];
		uint16_t len = (hw->SectorSize - off) < sizeof(buf) ? (hw->SectorSize - off) : sizeof(buf);
		hw->ReadBuffer(hw->Param, old * hw->SectorSize + off, buf, len);
		for (uint16_t i = 0; i < len; i++)
			for (int k = 0; k < 8; k++)
			{
				seed = seed * 1103515245u + 12345u;
				if (((seed >> 16) & 0xFF) < 51)		/* ~20% of the bits back to 1 */
					buf[i] |= (uint8_t)(1 << k);
			}
		hw->WriteBuffer(hw->Param, old * hw->SectorSize + off, buf, len);
	}

	/* reboot */
	hw->Synced = false;
	NorDB_t *DB2 = NorDB(hw, GetDummyRecordSize());
	if (DB2 == NULL)
	{
		printf("\tError: DB refused to open after interrupted erase\n");
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
	printf("\tNo consumed record delivered again, unread records intact and in order\n");

	/* keep writing across the interrupted sector, two full laps */
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

/* ---- IoFault_Test: inject a failing hw call at every interesting point ---- */
#define IOFAULT_MAX_OPS 4096

static NorDB_HWLayer *Io_hw;
static bool (*Io_orig_read)(void*, uint32_t, uint8_t*, uint16_t);
static bool (*Io_orig_write)(void*, uint32_t, uint8_t*, uint16_t);
static bool (*Io_orig_erase)(void*, uint32_t);
static uint32_t Io_calls;
static uint32_t Io_fail_at;		/* 0: count calls only */
static bool Io_fired;

static bool Io_Hit(void)
{
	Io_calls++;
	if (Io_fail_at != 0 && Io_calls == Io_fail_at)
	{
		Io_fired = true;
		return true;
	}
	return false;
}

static bool Io_Read(void *p, uint32_t a, uint8_t *d, uint16_t l)
{
	return Io_Hit() ? false : Io_orig_read(p, a, d, l);
}

static bool Io_Write(void *p, uint32_t a, uint8_t *d, uint16_t l)
{
	return Io_Hit() ? false : Io_orig_write(p, a, d, l);
}

static bool Io_Erase(void *p, uint32_t a)
{
	return Io_Hit() ? false : Io_orig_erase(p, a);
}

static void Io_Install(NorDB_HWLayer *hw, uint32_t fail_at)
{
	Io_hw = hw;
	Io_orig_read = hw->ReadBuffer;
	Io_orig_write = hw->WriteBuffer;
	Io_orig_erase = hw->SectorErace;
	hw->ReadBuffer = Io_Read;
	hw->WriteBuffer = Io_Write;
	hw->SectorErace = Io_Erase;
	Io_calls = 0;
	Io_fail_at = fail_at;
	Io_fired = false;
}

static void Io_Remove(void)
{
	Io_hw->ReadBuffer = Io_orig_read;
	Io_hw->WriteBuffer = Io_orig_write;
	Io_hw->SectorErace = Io_orig_erase;
}

typedef struct
{
	uint32_t n;
	uint32_t start[IOFAULT_MAX_OPS];	/* hw call counter before the op */
	uint32_t end[IOFAULT_MAX_OPS];		/* ... and after it */
	bool	 hot[IOFAULT_MAX_OPS];		/* op worth injecting faults into */
} IoRec_t;

static IoRec_t Io_rec;

typedef struct
{
	uint32_t w;		/* next sequence to write == records accepted */
	uint32_t r;		/* next sequence expected from a read */
	bool	 seen;	/* the injected fault was already handled */
} IoFault_t;

static void IoRec_Note(IoRec_t *rec, uint32_t start, uint32_t end, bool hot)
{
	if (rec == NULL || rec->n >= IOFAULT_MAX_OPS)
		return;
	rec->start[rec->n] = start;
	rec->end[rec->n] = end;
	rec->hot[rec->n] = hot || rec->n < 3;
	rec->n++;
}

static bool IoFault_Add(NorDB_t *DB, IoFault_t *st, IoRec_t *rec, uint32_t n)
{
	NorDB_HWLayer *hw = DB->DB_ll;
	dummy_t r;

	for (uint32_t done = 0; done < n; )
	{
		Fifo_MakeRecord(&r, st->w);
		uint32_t c0 = Io_calls;
		uint32_t sc = hw->SyncCounter;
		uint32_t adr = NorDB_AddRecord(DB, &r);
		IoRec_Note(rec, c0, Io_calls, adr == 0 || sc != hw->SyncCounter);
		if (adr == 0)
		{
			if (Io_fired && !st->seen)
			{
				st->seen = true;	/* injected fault: retry once it is gone */
				continue;
			}
			return true;			/* DB full */
		}
		st->w++;
		done++;
	}
	return true;
}

static bool IoFault_Read(NorDB_t *DB, IoFault_t *st, IoRec_t *rec, uint32_t n)
{
	NorDB_HWLayer *hw = DB->DB_ll;
	dummy_t r;

	for (uint32_t done = 0; done < n; )
	{
		uint32_t c0 = Io_calls;
		uint16_t lr = hw->LastReadSector;
		uint32_t adr = NorDB_ReadRecord(DB, &r);
		IoRec_Note(rec, c0, Io_calls, adr == 0 || lr != hw->LastReadSector);
		if (adr == 0)
		{
			if (st->r == st->w)
				return true;		/* empty */
			if (Io_fired && !st->seen)
			{
				st->seen = true;	/* injected fault: the record must still be there */
				continue;
			}
			printf("\tError: ReadRecord returned 0 with %u records pending\n", st->w - st->r);
			return false;
		}
		uint32_t seq = r.dummy[0] | (r.dummy[1] << 8) | (r.dummy[2] << 16) | ((uint32_t)r.dummy[3] << 24);
		if (check_dummyRecord(&r) == false || seq != st->r)
		{
			printf("\tError: read sequence %u, expected %u\n", seq, st->r);
			return false;
		}
		st->r++;
		done++;
	}
	return true;
}

/* aligned start, then a wrap: formats the sector after the writer and the one it came from */
static bool IoFault_Script(NorDB_t *DB, IoFault_t *st, IoRec_t *rec)
{
	uint32_t per = DB->Record_NumberInSector;
	uint32_t sectors = DB->DB_ll->SectorNumber;

	return IoFault_Add(DB, st, rec, 3) &&
		   IoFault_Read(DB, st, rec, 2) &&
		   IoFault_Add(DB, st, rec, per * (sectors - 1)) &&
		   IoFault_Read(DB, st, rec, per) &&
		   IoFault_Add(DB, st, rec, per) &&
		   IoFault_Read(DB, st, rec, UINT32_MAX);
}

static bool IoFault_Run(NorDB_t *DB, uint32_t fail_at)
{
	NorDB_HWLayer *hw = DB->DB_ll;
	IoFault_t st = {0, 0, false};
	uint32_t got = 0;
	bool ok = false;

	FreeRecord_Prepare(DB);
	uint32_t io_before = NorDB_Get_IoErrorCount(DB);

	/* the script drains everything it wrote, so stop it half way to keep records pending */
	Io_Install(hw, fail_at);
	uint32_t per = DB->Record_NumberInSector;
	uint32_t sectors = hw->SectorNumber;
	bool run = IoFault_Add(DB, &st, NULL, 3) &&
			   IoFault_Read(DB, &st, NULL, 2) &&
			   IoFault_Add(DB, &st, NULL, per * (sectors - 1)) &&
			   IoFault_Read(DB, &st, NULL, per) &&
			   IoFault_Add(DB, &st, NULL, per);
	bool fired = Io_fired;
	Io_Remove();
	if (!run)
	{
		printf("\tError: script failed with fault at call %u\n", fail_at);
		return false;
	}
	if (NorDB_Get_IoErrorCount(DB) != io_before + (fired ? 1 : 0))
	{
		printf("\tError: IoErrorCount %u, expected %u (fault at call %u)\n",
				NorDB_Get_IoErrorCount(DB), io_before + (fired ? 1 : 0), fail_at);
		return false;
	}
	if (NorDB_Get_TotalUnreadRecord(DB) != st.w - st.r)
	{
		printf("\tError: unread %u, expected %u (fault at call %u)\n",
				NorDB_Get_TotalUnreadRecord(DB), st.w - st.r, fail_at);
		return false;
	}

	/* reboot: everything accepted and not delivered must still be there, in order */
	hw->Synced = false;
	NorDB_t *DB2 = NorDB(hw, GetDummyRecordSize());
	if (DB2 == NULL)
	{
		printf("\tError: reopen failed (fault at call %u)\n", fail_at);
		return false;
	}
	if (NorDB_Get_TotalUnreadRecord(DB2) != st.w - st.r ||
		!Fifo_Read(DB2, &st.r, UINT32_MAX, &got) || st.r != st.w)
	{
		printf("\tError: records lost or reordered after reopen (fault at call %u)\n", fail_at);
		goto done;
	}
	ok = true;

done:
	NorDB_Destroy(DB2);
	NorDB_Clear(DB);
	return ok;
}

/* fail every hw call of an operation in turn; the op must report it, state must stay usable */
static bool IoFault_ClearOpenFormat(NorDB_t *DB)
{
	NorDB_HWLayer *hw = DB->DB_ll;
	uint16_t size = GetDummyRecordSize();

	for (uint32_t n = 1; ; n++)
	{
		IoFault_t st = {0, 0, false};
		FreeRecord_Prepare(DB);
		IoFault_Add(DB, &st, NULL, 4);
		uint32_t io_before = NorDB_Get_IoErrorCount(DB);
		Io_Install(hw, n);
		bool res = NorDB_Clear(DB);
		bool fired = Io_fired;
		Io_Remove();
		if (fired == res || NorDB_Get_IoErrorCount(DB) != io_before + (fired ? 1 : 0))
		{
			printf("\tError: Clear with fault at call %u: result %d fired %d\n", n, res, fired);
			return false;
		}
		if (!NorDB_Clear(DB) || NorDB_Get_TotalUnreadRecord(DB) != 0)
		{
			printf("\tError: Clear does not recover after fault at call %u\n", n);
			return false;
		}
		if (!fired)
			break;
	}
	printf("\tClear reports every io error and recovers\n");

	IoFault_t st = {0, 0, false};
	FreeRecord_Prepare(DB);
	IoFault_Add(DB, &st, NULL, 4);
	for (uint32_t n = 1; ; n++)
	{
		uint32_t io_before = NorDB_Get_IoErrorCount(DB);
		hw->Synced = false;
		Io_Install(hw, n);
		NorDB_t *DB2 = NorDB(hw, size);
		bool fired = Io_fired;
		Io_Remove();
		if ((!fired && DB2 == NULL) || NorDB_Get_IoErrorCount(DB) != io_before + (fired ? 1 : 0))
		{
			printf("\tError: NorDB() with fault at call %u: handle %p fired %d\n", n, (void*)DB2, fired);
			return false;
		}
		NorDB_Destroy(DB2);
		hw->Synced = false;
		DB2 = NorDB(hw, size);
		if (DB2 == NULL || NorDB_Get_TotalUnreadRecord(DB2) != 4)
		{
			printf("\tError: NorDB() does not recover after fault at call %u\n", n);
			NorDB_Destroy(DB2);
			return false;
		}
		NorDB_Destroy(DB2);
		if (!fired)
			break;
	}
	printf("\tNorDB() fails cleanly on io errors and recovers\n");

	for (uint32_t n = 1; ; n++)
	{
		Io_Install(hw, n);
		bool res = NorDB_Format(hw);
		bool fired = Io_fired;
		Io_Remove();
		if (fired == res)
		{
			printf("\tError: Format with fault at call %u: result %d fired %d\n", n, res, fired);
			return false;
		}
		if (!fired)
			break;
		if (!NorDB_Format(hw))
		{
			printf("\tError: Format does not recover after fault at call %u\n", n);
			return false;
		}
	}
	NorDB_t *DB3 = NorDB(hw, size);
	if (DB3 == NULL || NorDB_Get_TotalUnreadRecord(DB3) != 0)
	{
		printf("\tError: DB not empty after Format\n");
		NorDB_Destroy(DB3);
		return false;
	}
	NorDB_Destroy(DB3);
	printf("\tFormat reports every io error and recovers\n");
	return true;
}

bool IoFault_Test(NorDB_t *DB, char *name, int count)
{
	(void)count;
	strcpy(name, "IO Fault");

	printf("--->NorDB IO Fault Test\n");

	NorDB_HWLayer *hw = DB->DB_ll;
	IoFault_t st = {0, 0, false};

	/* dry run: count the hw calls of every op, pick the ones around sector changes */
	Io_rec.n = 0;
	FreeRecord_Prepare(DB);
	Io_Install(hw, 0);
	bool ok = IoFault_Script(DB, &st, &Io_rec);
	Io_Remove();
	if (!ok || st.r != st.w || Io_rec.n == 0)
	{
		printf("\tError: dry run failed\n");
		NorDB_Clear(DB);
		return false;
	}
	Io_rec.hot[Io_rec.n - 1] = true;

	uint32_t runs = 0;
	for (uint32_t i = 0; i < Io_rec.n; i++)
	{
		if (!Io_rec.hot[i])
			continue;
		for (uint32_t at = Io_rec.start[i] + 1; at <= Io_rec.end[i]; at++)
		{
			if (!IoFault_Run(DB, at))
				return false;
			runs++;
		}
	}
	printf("\t%u injected faults: nothing lost, reordered or delivered twice\n", runs);

	if (!IoFault_ClearOpenFormat(DB))
		return false;

	NorDB_Clear(DB);
	printf("\n");
	return true;
}

/* ---- AmbiguousIoFault_Test: writes can report failure after reaching flash ---- */
static NorDB_HWLayer *Amb_hw;
static bool (*Amb_orig_write)(void*, uint32_t, uint8_t*, uint16_t);
static bool (*Amb_orig_erase)(void*, uint32_t);
static uint32_t Amb_mutating_calls;
static uint32_t Amb_fail_at;
static bool Amb_fired;
static uint8_t Amb_record[255];

static bool Amb_Write(void *p, uint32_t a, uint8_t *d, uint16_t l)
{
	bool result = Amb_orig_write(p, a, d, l);
	Amb_mutating_calls++;
	if(Amb_fail_at != 0 && Amb_mutating_calls == Amb_fail_at)
	{
		Amb_fired = true;
		Amb_fail_at = 0;
		return false;
	}
	return result;
}

static bool Amb_Erase(void *p, uint32_t a)
{
	bool result = Amb_orig_erase(p, a);
	Amb_mutating_calls++;
	if(Amb_fail_at != 0 && Amb_mutating_calls == Amb_fail_at)
	{
		Amb_fired = true;
		Amb_fail_at = 0;
		return false;
	}
	return result;
}

static void Amb_Install(NorDB_HWLayer *hw, uint32_t fail_at)
{
	Amb_hw = hw;
	Amb_orig_write = hw->WriteBuffer;
	Amb_orig_erase = hw->SectorErace;
	hw->WriteBuffer = Amb_Write;
	hw->SectorErace = Amb_Erase;
	Amb_mutating_calls = 0;
	Amb_fail_at = fail_at;
	Amb_fired = false;
}

static void Amb_Remove(void)
{
	Amb_hw->WriteBuffer = Amb_orig_write;
	Amb_hw->SectorErace = Amb_orig_erase;
}

static void Amb_MakeRecord(uint16_t size, uint32_t seq)
{
	for(uint16_t i = 0; i < size; i++)
		Amb_record[i] = (uint8_t)(seq * 7u + i);
	for(uint16_t i = 0; i < 4 && i < size; i++)
		Amb_record[i] = (uint8_t)(seq >> (8 * i));
}

static bool Amb_CheckRecord(uint16_t size, uint32_t seq)
{
	uint32_t actual = Amb_record[0] | ((uint32_t)Amb_record[1] << 8) |
			((uint32_t)Amb_record[2] << 16) | ((uint32_t)Amb_record[3] << 24);
	if(actual != seq)
		return false;
	for(uint16_t i = 4; i < size; i++)
		if(Amb_record[i] != (uint8_t)(seq * 7u + i))
			return false;
	return true;
}

static bool Ambiguous_Run(NorDB_t *DB, uint32_t fail_at, uint32_t *mutating_calls)
{
	NorDB_HWLayer *hw = DB->DB_ll;
	NorDB_t *test_db = NULL;
	NorDB_t *clean_db = NULL;
	bool installed = false;
	bool ok = false;
	uint32_t per_sector = 0;
	uint32_t records = 0;
	uint32_t next_write = 0;
	uint32_t next_read = 0;
	uint32_t pending = 0;
	uint16_t payload_size = (uint16_t)((hw->SectorSize - sizeof(NorDB_Header_t) - 1) / 6);
	if(payload_size > sizeof(Amb_record))
		payload_size = sizeof(Amb_record);
	if(payload_size < 4)
		payload_size = 4;

	if(!NorDB_Format(hw))
		goto done;
	test_db = NorDB(hw, payload_size);
	if(test_db == NULL)
		goto done;
	Amb_Install(hw, fail_at);
	installed = true;
	per_sector = test_db->Record_NumberInSector;
	records = per_sector * hw->SectorNumber;
	for(uint32_t lap = 0; lap < 2; lap++)
	{
		for(uint32_t i = 0; i < records; i++)
		{
			bool added = false;
			for(uint32_t attempt = 0; attempt < 4; attempt++)
			{
				Amb_MakeRecord(payload_size, next_write);
				if(NorDB_AddRecord(test_db, Amb_record) != 0)
				{
					added = true;
					next_write++;
					pending++;
					break;
				}
				if(NorDB_Get_TotalUnreadRecord(test_db) != pending)
					goto done;
			}
			if(!added || NorDB_Get_TotalUnreadRecord(test_db) != pending)
				goto done;

			if(pending > 0)
			{
				bool read = false;
				for(uint32_t attempt = 0; attempt < 4; attempt++)
				{
					if(NorDB_ReadRecord(test_db, Amb_record) != 0)
					{
						read = true;
						break;
					}
					if(NorDB_Get_TotalUnreadRecord(test_db) != pending)
						goto done;
				}
				if(!read || !Amb_CheckRecord(payload_size, next_read))
					goto done;
				next_read++;
				pending--;
			}
			if(NorDB_Get_TotalUnreadRecord(test_db) != pending)
				goto done;
		}
	}
	while(pending > 0)
	{
		bool read = false;
		for(uint32_t attempt = 0; attempt < 4; attempt++)
		{
			if(NorDB_ReadRecord(test_db, Amb_record) != 0)
			{
				read = true;
				break;
			}
			if(NorDB_Get_TotalUnreadRecord(test_db) != pending)
				goto done;
		}
		if(!read || !Amb_CheckRecord(payload_size, next_read))
			goto done;
		next_read++;
		pending--;
		if(NorDB_Get_TotalUnreadRecord(test_db) != pending)
			goto done;
	}

	/* A failed add is never delivered: it leaves the slot read or never marked. */
	ok = next_read == next_write && pending == 0 &&
		NorDB_Get_TotalUnreadRecord(test_db) == 0 &&
		Amb_fired == (fail_at != 0);

done:
	if(installed)
		Amb_Remove();
	if(mutating_calls != NULL)
		*mutating_calls = Amb_mutating_calls;
	NorDB_Destroy(test_db);
	if(!NorDB_Format(hw))
		ok = false;
	else
	{
		clean_db = NorDB(hw, GetDummyRecordSize());
		if(clean_db == NULL)
			ok = false;
		else
			NorDB_Destroy(clean_db);
	}
	return ok;
}

bool AmbiguousIoFault_Test(NorDB_t *DB, char *name, int count)
{
	strcpy(name, "Ambiguous IO Fault");

	printf("--->NorDB Ambiguous IO Fault Test\n");
	uint32_t calls = 0;
	uint32_t stride = count > 0 ? (uint32_t)count : 1;
	if(!Ambiguous_Run(DB, 0, &calls) || calls == 0)
	{
		printf("\tError: workload dry run failed\n");
		return false;
	}
	uint32_t positions = 0;
	for(uint32_t i = 1; i <= calls; i += stride)
	{
		if(!Ambiguous_Run(DB, i, NULL))
		{
			printf("\tError: ambiguous mutation at call %u failed\n", i);
			return false;
		}
		positions++;
	}
	if((calls - 1) % stride != 0 && !Ambiguous_Run(DB, calls, NULL))
	{
		printf("\tError: ambiguous mutation at call %u failed\n", calls);
		return false;
	}
	if((calls - 1) % stride != 0)
		positions++;
	printf("\tVerified %u ambiguous mutations across %u fault positions (stride %u)\n\n",
		calls, positions, stride);
	return true;
}

int RunTest(bool(*test)(NorDB_t*, char*, int), NorDB_t* DB, int count)
{
    char testName[20] = {0};
    double start, end;

    tests_run++;
    start = GetTime();
    if(!test(DB, testName, count))
	{
		printf("%s Test Not Complete\n", testName);
		tests_failed++;
		return EXIT_FAILURE;
	}
    end = GetTime();
    double time_spent = end - start;
    printf("Test Execution Time: %f%cs\n\n", time_spent<0.1f?(time_spent*1000.0f):time_spent, time_spent<0.1f?'m':0);
    return 0;
}

int TestSummary(void)
{
	if (tests_failed == 0)
		printf("All %d tests passed\n", tests_run);
	else
		printf("Tests failed: %d of %d\n", tests_failed, tests_run);
	return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

void RunFlashTests(NorDB_t *DB, int ambiguous_stride)
{
	RunTest(FullFill_Test, DB, 1);
	RunTest(RoundRobin_Test, DB, 1);
	RunTest(OverWrite_Test, DB, 1);
	RunTest(WriteRead_Time_Test, DB, 100);
	RunTest(CRC_DetectsCorruption_Test, DB, 0);
	RunTest(PowerLoss_PhantomRecord_Test, DB, 0);
	RunTest(ErasedCrc_Test, DB, 0);
	RunTest(FreeRecord_Accuracy_Test, DB, 0);
	RunTest(IncompatibleHeader_Test, DB, 0);
	RunTest(FifoOrder_Wrap_Test, DB, 0);
	RunTest(PowerLoss_SectorHeader_Test, DB, 0);
	RunTest(PowerLoss_Erase_Test, DB, 0);
	RunTest(InitParams_Test, DB, 0);
	RunTest(IoFault_Test, DB, 0);
	RunTest(AmbiguousIoFault_Test, DB, ambiguous_stride);
	RunTest(DeleteDB_Test, DB, 100);
	RunTest(Clear_Test, DB, 100);
	RunTest(ReadEmpty_Test, DB, 0);
}
