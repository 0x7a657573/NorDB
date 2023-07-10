/*
 * NorDB.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */

#ifndef LightDB_LightDB_H_
#define LightDB_LightDB_H_
#include "NorDB_def.h"
#include "sem_ll.h"

typedef struct
{
	NorDB_sem_t	sema;   // semaphore for working with DB
	uint16_t SectorSize;    // Sector size of the hardware
	uint16_t SectorNumber;  // Total number of sectors the HW has

	/*statistics var*/
	uint32_t TotalUnreadRecord; 

	/*Read/Write Sync*/
	bool	 Synced;
	uint32_t SyncCounter;
	uint16_t LastWriteSector;
	uint16_t LastReadSector;

	void 	*Param;
	void 	(*Erase)(void*Param);
	void 	(*SectorErace)(void*Param,uint32_t address);
	void 	(*WriteBuffer)(void*Param,uint32_t address,uint8_t *data,uint16_t len);
	void    (*ReadBuffer)(void*Param,uint32_t address,uint8_t *data,uint16_t len);
	uint8_t (*DriverCheck)(void*Param);
	const char *(*DriverName)(void*Param);
}NorDB_HWLayer;


typedef struct
{
	NorDB_HWLayer *DB_ll;	//HW layer for DB

	/*hw interface cache*/

	/*database var*/
	uint8_t  *Header_Cache;
	uint16_t Header_Size;			// Total length of a header in a sector
	uint16_t Record_Size;			// length of data + crc for a block
	uint16_t Record_NumberInSector;
}NorDB_t;

typedef struct __attribute__((__packed__))
{
	uint16_t Magic;	//Signature
	uint16_t Vertion;
	uint32_t SyncCounter;
	uint16_t RecordSize;
	uint8_t  Records[];
}NorDB_Header_t;


NorDB_t *NorDB(NorDB_HWLayer *hw,uint16_t RecordSize);
uint32_t NorDB_AddRecord(NorDB_t *db,void *RecoedData);
uint32_t NorDB_ReadRecord(NorDB_t *db,void *RecoedData);
uint32_t NorDB_Get_TotalUnreadRecord(NorDB_t *db);
uint32_t NorDB_Get_FreeRecord(NorDB_t *db);

#endif /* LightDB_LightDB_H_ */
