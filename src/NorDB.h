/*
 * NorDB.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */

#ifndef LightDB_LightDB_H_
#define LightDB_LightDB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "NorDB_def.h"
#include "sem_ll.h"

typedef struct
{
	NorDB_sem_t	sema;
	uint16_t SectorSize;
	uint16_t SectorNumber;

	/*statistics var*/
	uint32_t TotalUnreadRecord;

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
	NorDB_HWLayer *DB_ll;

	/*hw interface cache*/

	/*database var*/
	uint8_t  *Header_Cache;
	uint16_t Header_Size;
	uint16_t Record_Size;
	uint16_t Record_NumberInSector;
}NorDB_t;

typedef struct __attribute__((__packed__))
{
	uint16_t Magic;
	uint16_t RecordSize;
	uint8_t  Records[];
}NorDB_Header_t;


NorDB_t *NorDB(NorDB_HWLayer *hw,uint16_t RecordSize);
uint32_t NorDB_AddRecord(NorDB_t *db,void *RecoedData);
uint32_t NorDB_ReadRecord(NorDB_t *db,void *RecoedData);
uint32_t NorDB_Get_TotalUnreadRecord(NorDB_t *db);

#endif /* LightDB_LightDB_H_ */
