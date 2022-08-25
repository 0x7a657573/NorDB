/*
 * FileDB.h
 *
 *  Created on: Feb 16, 2019
 *      Author: Mazarei
 */

#ifndef FILEDB_H_
#define FILEDB_H_
#include "LightDB.h"
#include <stdbool.h>

#define DBFileName		"point.db"
#define DBSectorSize	4096
typedef uint32_t		SemaphoreHandle_t;

typedef struct
{
	LightDB_HWLayer 	*LightDB;
	int32_t				FileHandle;
	bool				Enable;
	uint8_t				Sector[DBSectorSize];
}FileDB_Param;

LightDB_HWLayer *FileDB_Init(uint16_t RecordSize,uint16_t TotalSector);


#endif /* FILEDB_H_ */
