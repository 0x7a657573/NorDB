/*
 * Ram_ll.h
 *
 *  Created on: Jul 18, 2021
 *      Author: zeus
 */

#ifndef RAMDB_RAMDB_H_
#define RAMDB_RAMDB_H_

#include "../NorDB.h"




typedef struct
{
	 uint16_t 		Sector_Size;
	 uint16_t 		Total_Sector;
	 uint32_t		Total_Size;
	 uint8_t  		*Buffer;
}Ramll_t;

NorDB_HWLayer *Ramll_Init(uint16_t SectorSize,uint16_t TotalSector);
void Ramll_Del(NorDB_HWLayer *db);


#endif /* RAMDB_RAMDB_H_ */
