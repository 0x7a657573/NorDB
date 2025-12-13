/**********************************************************************
 * File : File_ll.h.h
 * Copyright (c) 0x7a657573.
 * Created On : Sat Aug 27 2022
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#ifndef File_ll_h
#define File_ll_h
#include <stdint.h>
#include <stdlib.h>
#include "../NorDB.h"

typedef struct
{
	 nordb_FILE     *fileid;
     uint16_t 		Sector_Size;
	 uint16_t 		Total_Sector;
	 uint32_t		Total_Size;
}Filell_t;

NorDB_HWLayer *Filell_Init(char *path,uint16_t SectorSize,uint16_t TotalSector);
void Filell_Erase(void *Param);
void Filell_Del(NorDB_HWLayer *fdb);


#endif /* File_ll.h */
