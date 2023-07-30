/*
 * NorDB.c
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */

#include "NorDB.h"
#include <string.h>


#define nordb_GetBitStatus(bytes,index) ((bytes[index/4]>>((index*2)%8))&0b11)

#define Is_TrueHeader(db,Header) 	(Header->Magic==NorDB_Magic && \
									 Header->Vertion==NorDB_RVer && \
									 Header->RecordSize==db->Record_Size)

void NorDB_SyncData(NorDB_t *db);

static uint8_t crc8x_table[] =
{
    0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97, 0xb9, 0x88, 0xdb, 0xea, 0x7d,
    0x4c, 0x1f, 0x2e, 0x43, 0x72, 0x21, 0x10, 0x87, 0xb6, 0xe5, 0xd4, 0xfa, 0xcb,
    0x98, 0xa9, 0x3e, 0x0f, 0x5c, 0x6d, 0x86, 0xb7, 0xe4, 0xd5, 0x42, 0x73, 0x20,
    0x11, 0x3f, 0x0e, 0x5d, 0x6c, 0xfb, 0xca, 0x99, 0xa8, 0xc5, 0xf4, 0xa7, 0x96,
    0x01, 0x30, 0x63, 0x52, 0x7c, 0x4d, 0x1e, 0x2f, 0xb8, 0x89, 0xda, 0xeb, 0x3d,
    0x0c, 0x5f, 0x6e, 0xf9, 0xc8, 0x9b, 0xaa, 0x84, 0xb5, 0xe6, 0xd7, 0x40, 0x71,
    0x22, 0x13, 0x7e, 0x4f, 0x1c, 0x2d, 0xba, 0x8b, 0xd8, 0xe9, 0xc7, 0xf6, 0xa5,
    0x94, 0x03, 0x32, 0x61, 0x50, 0xbb, 0x8a, 0xd9, 0xe8, 0x7f, 0x4e, 0x1d, 0x2c,
    0x02, 0x33, 0x60, 0x51, 0xc6, 0xf7, 0xa4, 0x95, 0xf8, 0xc9, 0x9a, 0xab, 0x3c,
    0x0d, 0x5e, 0x6f, 0x41, 0x70, 0x23, 0x12, 0x85, 0xb4, 0xe7, 0xd6, 0x7a, 0x4b,
    0x18, 0x29, 0xbe, 0x8f, 0xdc, 0xed, 0xc3, 0xf2, 0xa1, 0x90, 0x07, 0x36, 0x65,
    0x54, 0x39, 0x08, 0x5b, 0x6a, 0xfd, 0xcc, 0x9f, 0xae, 0x80, 0xb1, 0xe2, 0xd3,
    0x44, 0x75, 0x26, 0x17, 0xfc, 0xcd, 0x9e, 0xaf, 0x38, 0x09, 0x5a, 0x6b, 0x45,
    0x74, 0x27, 0x16, 0x81, 0xb0, 0xe3, 0xd2, 0xbf, 0x8e, 0xdd, 0xec, 0x7b, 0x4a,
    0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xc2, 0xf3, 0xa0, 0x91, 0x47, 0x76, 0x25,
    0x14, 0x83, 0xb2, 0xe1, 0xd0, 0xfe, 0xcf, 0x9c, 0xad, 0x3a, 0x0b, 0x58, 0x69,
    0x04, 0x35, 0x66, 0x57, 0xc0, 0xf1, 0xa2, 0x93, 0xbd, 0x8c, 0xdf, 0xee, 0x79,
    0x48, 0x1b, 0x2a, 0xc1, 0xf0, 0xa3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49,
    0x1a, 0x2b, 0xbc, 0x8d, 0xde, 0xef, 0x82, 0xb3, 0xe0, 0xd1, 0x46, 0x77, 0x24,
    0x15, 0x3b, 0x0a, 0x59, 0x68, 0xff, 0xce, 0x9d, 0xac
};

unsigned NorDB_crc8x_fast(void const *mem, size_t len)
{
	unsigned crc = 0xFF;
    unsigned char const *data = mem;
    if (data == NULL)
        return 0xff;
    crc &= 0xff;
    while (len--)
        crc = crc8x_table[crc ^ *data++];

    return crc;
}

NorDB_t *NorDB(NorDB_HWLayer *hw,uint16_t RecordSize)
{
	if(!hw)
		return NULL;
	NorDB_t *DB = nordb_malloc(sizeof(NorDB_t));
	if(!DB)
		return NULL;
	
	nordb_memset(DB,0,sizeof(NorDB_t));
	DB->DB_ll = hw;
	DB->Record_Size = RecordSize + sizeof(uint8_t);	/*add one byte crc*/
	if((DB->Record_Size+sizeof(NorDB_Header_t)+1) > hw->SectorSize)
	{
		nordb_free(DB);
		return NULL;
	}
	
	uint32_t max_record_in_sector = (((hw->SectorSize - (sizeof(NorDB_Header_t))) / DB->Record_Size) + 1);
	uint32_t header_space_for_status = max_record_in_sector / 4 + 1;
	DB->Header_Size = header_space_for_status + sizeof(NorDB_Header_t);
	DB->Record_NumberInSector = (hw->SectorSize - DB->Header_Size) / DB->Record_Size;
	DB->Header_Cache = (uint8_t*)nordb_malloc(DB->Header_Size);

	if(!hw->Synced)
	{
		NorDB_SyncData(DB);
	}
	
	return DB;
}

bool NorDB_Init_Sector(NorDB_t *db, int Sector)
{
	NorDB_HWLayer *hw = db->DB_ll;

	/*check sector validate*/
	if(Sector >= hw->SectorNumber)
		return false;

	/*erase sector*/
	hw->SectorErace(hw->Param,Sector*hw->SectorSize);
	
	hw->SyncCounter++;
	NorDB_Header_t Header;
	Header.Magic 	= NorDB_Magic;
	Header.Vertion	= NorDB_RVer;
	Header.SyncCounter = hw->SyncCounter;
	Header.RecordSize = db->Record_Size;

	/*write header*/
	hw->WriteBuffer(hw->Param,(Sector*hw->SectorSize),(uint8_t*)&Header,sizeof(NorDB_Header_t));
	return true;
}

uint32_t NorDB_Find_First_Free_point_in_Sector(NorDB_t *db, int Start_Sector_Search,NorDB_Header_t *Header,bool canformatinused)
{
  if(db==NULL)	return 0;
  NorDB_HWLayer *hw = db->DB_ll;

  /*read header*/
  hw->ReadBuffer(hw->Param,(Start_Sector_Search*hw->SectorSize),(uint8_t*)Header,db->Header_Size);

  /*not init sector*/
  if(!Is_TrueHeader(db,Header))
  {
	  if(NorDB_Init_Sector(db, Start_Sector_Search))
		  return (Start_Sector_Search*hw->SectorSize) + db->Header_Size;
	  return 0;
  }

  bool its_used = true;
  for(int i=0;i<db->Record_NumberInSector;i++)
  {
	  uint8_t Record_status = nordb_GetBitStatus(Header->Records,i);
	  /*if record is free*/
	  if(Record_status == nordb_FreeMark)
	  {
		  return (Start_Sector_Search*hw->SectorSize) + db->Header_Size + (i * (db->Record_Size));
	  }

 	  if(Record_status != nordb_ReadMark)
	  {
		its_used = false;
	  }
  }
  
  /*we can use this sector*/
  if(its_used && canformatinused)
  {
	if(NorDB_Init_Sector(db, Start_Sector_Search))
		return (Start_Sector_Search*hw->SectorSize) + db->Header_Size;
	return 0;
  }

  /*not found*/
  return 0;
}

uint32_t NorDB_GetWriteable_Record(NorDB_t *db)
{
	if(db==NULL)
		return 0;

	NorDB_Header_t *Header = (NorDB_Header_t*) db->Header_Cache;
	bool CanForamtInUsed = false;
	uint16_t SearchSector = db->DB_ll->LastWriteSector;
	for(int i=0;i<2;i++)
	{
		uint32_t res = NorDB_Find_First_Free_point_in_Sector(db, SearchSector,Header,CanForamtInUsed);
		if(res!=0)
		{
			db->DB_ll->LastWriteSector = SearchSector;
			return res;
		}
		/*we can not found free pos try next sector*/	
		SearchSector = (++SearchSector) >= db->DB_ll->SectorNumber ? 0:SearchSector;
		CanForamtInUsed = true;
	}
	return 0;
}

void NorDB_SyncData(NorDB_t *db)
{
	if(db==NULL)
		return;

	uint32_t UnreadRecord = 0;

	NorDB_HWLayer *hw = db->DB_ll;
	/*lock io*/
	NorDB_sem_Lock(&hw->sema);
	NorDB_Header_t *Header = (NorDB_Header_t*) db->Header_Cache;
	hw->SyncCounter = 0;
	uint32_t LastRead = UINT32_MAX;
	hw->LastWriteSector = 0;
	hw->LastReadSector =0;
	for(uint32_t i=0; i< db->DB_ll->SectorNumber; i++)
	{
		/*read header*/
		hw->ReadBuffer(hw->Param,(i*hw->SectorSize),(uint8_t*)Header,db->Header_Size);

		/*not init sector*/
		if(!Is_TrueHeader(db,Header))
		{
			continue;
		}

		bool HasUnread = false;
		for(int j=0;j<db->Record_NumberInSector;j++)
		{
		  /*if record is free*/
		  if(nordb_GetBitStatus(Header->Records,j) == nordb_UnReadMark)
		  
		  {
			HasUnread = true;
			UnreadRecord++;
		  }
		}

		/*this sector has unread record ?*/
		if(HasUnread)
		{
			/*find lower number of syncCounter*/
			if(LastRead > Header->SyncCounter)
			{
				LastRead = Header->SyncCounter;
				hw->LastReadSector = i;
			}
		}

		/*find last sync number*/
		if(hw->SyncCounter < Header->SyncCounter)
		{
			hw->SyncCounter = Header->SyncCounter;
			hw->LastWriteSector = i;
		}
	}

	if(UnreadRecord==0)
	{
		hw->LastReadSector = hw->LastWriteSector;
	}

	hw->TotalUnreadRecord = UnreadRecord;
	hw->Synced = true;
	/*unlock io*/
	NorDB_sem_Unlock(&hw->sema);
}

uint32_t NorDB_Find_First_Unread_point_in_Sector(NorDB_t *db ,uint32_t Start_Sector_Search,NorDB_Header_t *Header)
{
	if(db==NULL)
		return 0;

	NorDB_HWLayer *hw = db->DB_ll;

	/*read header*/
	hw->ReadBuffer(hw->Param,(Start_Sector_Search*hw->SectorSize),(uint8_t*)Header,db->Header_Size);

	/*not init sector*/
	if(!Is_TrueHeader(db,Header))
	{
		return 0;
	}


	for(int i=0;i<db->Record_NumberInSector;i++)
	{
	  /*if record is free*/
	  if(nordb_GetBitStatus(Header->Records,i) == nordb_UnReadMark)
	  {
		  return (Start_Sector_Search*hw->SectorSize) + db->Header_Size + (i * (db->Record_Size));
	  }
	}

	return 0;
}

uint32_t NorDB_GetReadable_Record(NorDB_t *db)
{
	if(db==NULL)
		return 0;

	if(db->DB_ll->TotalUnreadRecord==0)
		return 0;

	NorDB_HWLayer *hw = db->DB_ll;
	NorDB_Header_t *Header = (NorDB_Header_t*) db->Header_Cache;
	uint16_t TotalSearchSector = (hw->LastReadSector < hw->LastWriteSector) ? 
								 (hw->LastWriteSector - hw->LastReadSector):
								 ((hw->SectorNumber - hw->LastReadSector) + hw->LastWriteSector);
	info_log("R->LastWrite:%i, LastRead:%i, TotalSearch:%i\r\n",hw->LastWriteSector,
				hw->LastReadSector,TotalSearchSector);
	for(int i=hw->LastReadSector;i<=hw->LastReadSector+TotalSearchSector;i++)
	{
		uint16_t SearchSector = (i >= hw->SectorNumber) ? (i-hw->SectorNumber):i;
		uint32_t res = NorDB_Find_First_Unread_point_in_Sector(db, SearchSector,Header);
		info_log("   -> Current:%i -> %i\r\n",SearchSector,res);
		if(res!=0)
		{
			hw->LastReadSector = SearchSector;
			return res;
		}
		/*we can not found free pos try next sector*/	
	}
	
	err_log("   Err-> Can Not Found(unRead:%i).\r\n",hw->TotalUnreadRecord);
	/*need update unread point :|*/
	/*maybe we have bad sector*/
	//hw->TotalUnreadRecord=0;

	return 0;
}

void NorDB_Set_Write_Header_In_sector(NorDB_t *db,uint32_t Point_Adr)
{
  if(db==NULL)
	  return;
  NorDB_HWLayer *hw = db->DB_ll;
  uint32_t SecNumber = Point_Adr/hw->SectorSize;
  Point_Adr%=hw->SectorSize;
  int RecordIndex = ((Point_Adr - db->Header_Size)/db->Record_Size);
  int ByteAddress = (RecordIndex / 4) + sizeof(NorDB_Header_t);
  int bit_shift   = ((RecordIndex%4)*2);

  uint8_t data = ~(nordb_FreeMark<<bit_shift);
  data |= nordb_UnReadMark << bit_shift;
  hw->WriteBuffer(hw->Param,SecNumber*hw->SectorSize + ByteAddress,&data,1);
}

void NorDB_Set_Read_Header_In_sector(NorDB_t *db,uint32_t Point_Adr)
{
	if(db==NULL)
		return;
	NorDB_HWLayer *hw = db->DB_ll;

	uint32_t SecNumber = Point_Adr/hw->SectorSize;
	Point_Adr%=hw->SectorSize;
  	int RecordIndex = ((Point_Adr - db->Header_Size)/db->Record_Size);
  	int ByteAddress = (RecordIndex / 4) + sizeof(NorDB_Header_t);
  	int bit_shift   = ((RecordIndex%4)*2);

  	uint8_t data = ~(nordb_FreeMark<<bit_shift);
  	data |= nordb_ReadMark << bit_shift;
  	hw->WriteBuffer(hw->Param,SecNumber*hw->SectorSize + ByteAddress,&data,1);
}

uint32_t NorDB_AddRecord(NorDB_t *db,void *RecoedData)
{
	if(db==NULL)
		return 0;

	NorDB_HWLayer *hw = db->DB_ll;

	/*lock io*/
	NorDB_sem_Lock(&hw->sema);


	uint8_t Temp_Buffer[db->Record_Size];

 	uint32_t Record = NorDB_GetWriteable_Record(db);
	if(Record==0)
	{
		/*unlock io*/
		NorDB_sem_Unlock(&hw->sema);
		return 0;
	}

	NorDB_Set_Write_Header_In_sector(db,Record);


	uint8_t Record_Crc = NorDB_crc8x_fast(RecoedData,db->Record_Size-1);
	nordb_memcpy(Temp_Buffer,RecoedData,db->Record_Size-1);
	Temp_Buffer[db->Record_Size-1] = Record_Crc;
	hw->WriteBuffer(hw->Param,Record,Temp_Buffer,db->Record_Size);


	nordb_memset((char*)Temp_Buffer,0,db->Record_Size);
	hw->ReadBuffer(hw->Param,Record,Temp_Buffer,db->Record_Size);
	uint8_t ReadBack_Crc = NorDB_crc8x_fast(Temp_Buffer,db->Record_Size-1);

	if(Record_Crc!=ReadBack_Crc) /*Read Back Data Faild*/
	{
		/*Mark Az Read*/
		NorDB_Set_Read_Header_In_sector(db,Record);
		/*unlock io*/
		NorDB_sem_Unlock(&hw->sema);
		return 0;
	}


	if(Record_Crc!=Temp_Buffer[db->Record_Size-1]) /*Read Back Data Faild*/
	{
		NorDB_Set_Read_Header_In_sector(db,Record); /*Mark Az Read*/
		/*unlock io*/
		NorDB_sem_Unlock(&hw->sema);
		return 0;
	}

	hw->TotalUnreadRecord++;

	/*unlock io*/
	NorDB_sem_Unlock(&hw->sema);
	return Record;
}

uint32_t NorDB_ReadRecord(NorDB_t *db,void *RecoedData)
{
	if(db==NULL)
		return 0;

	NorDB_HWLayer *hw = db->DB_ll;

	/*lock io*/
	NorDB_sem_Lock(&hw->sema);

	uint32_t Record = NorDB_GetReadable_Record(db);
	if(Record==0)
	{
		/*unlock io*/
		NorDB_sem_Unlock(&hw->sema);
		return 0;
	}

	uint8_t Temp_Buffer[db->Record_Size];
	uint16_t Record_Crc = 0;
	hw->ReadBuffer(hw->Param,Record,Temp_Buffer,db->Record_Size);
	Record_Crc = Temp_Buffer[db->Record_Size-1];
	NorDB_Set_Read_Header_In_sector(db,Record);
	hw->TotalUnreadRecord--;

	uint8_t Readback_Crc = NorDB_crc8x_fast(Temp_Buffer,db->Record_Size-1);

	if(Record_Crc!=Readback_Crc)
	{
		/*unlock io*/
		NorDB_sem_Unlock(&hw->sema);
		return 0; /*Not Valid Crc*/
	}

	nordb_memcpy(RecoedData,Temp_Buffer,db->Record_Size-1);
	/*unlock io*/
	NorDB_sem_Unlock(&hw->sema);
	return Record;
}

const char *NorDB_HwName(NorDB_t *db)
{
	if(db==NULL)	return 0;
	NorDB_HWLayer *hw = db->DB_ll;
	return hw->DriverName(hw->Param);
}

bool NorDB_HwCheck(NorDB_t *db)
{
	if(db==NULL)	return 0;
	NorDB_HWLayer *hw = db->DB_ll;
	/*lock io*/
	NorDB_sem_Lock(&hw->sema);
	bool res = hw->DriverCheck(hw->Param) ? true:false;
	/*unlock io*/
	NorDB_sem_Unlock(&hw->sema);

	return res;
}

uint32_t NorDB_Get_TotalUnreadRecord(NorDB_t *db)
{

	if(db==NULL)	return 0;

	NorDB_HWLayer *hw = db->DB_ll;

	/*lock io*/
	NorDB_sem_Lock(&hw->sema);

	uint32_t unread = hw->TotalUnreadRecord;

	/*unlock io*/
	NorDB_sem_Unlock(&hw->sema);

	return unread;
}

uint32_t NorDB_Get_FreeRecord(NorDB_t *db)
{
	if(db==NULL)	return 0;

	NorDB_HWLayer *hw = db->DB_ll;

	/*lock io*/
	NorDB_sem_Lock(&hw->sema);

	uint32_t TotalRecord = hw->SectorNumber * db->Record_NumberInSector;
	uint32_t unread = hw->TotalUnreadRecord;

	/*unlock io*/
	NorDB_sem_Unlock(&hw->sema);

	return TotalRecord - unread;
}
