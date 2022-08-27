/**********************************************************************
 * File : File_ll.c
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
#include "ll/File_ll.h"

void Filell_Erase(void *Param)
{
	Filell_t *ptr = Param;
    if(nordb_fseek(ptr->fileid, 0, SEEK_SET)!=0)
        return;

    char tmp = 0xFF;
    for(int i=0;i<ptr->Total_Size;i++)
        nordb_fwrite(ptr->fileid,&tmp,1);
}

void Filell_SectorErace(void*Param,uint32_t address)
{
	Filell_t *ptr = Param;

	uint16_t Sector_Number  = address / ptr->Sector_Size;
	if(Sector_Number < ptr->Total_Sector)
    {
        /*remove offset of sector :{*/
        if(nordb_fseek(ptr->fileid, Sector_Number*ptr->Sector_Size, SEEK_SET)!=0)
            return;
        
        char tmp = 0xFF;
        for(int i=0;i<ptr->Sector_Size;i++)
            nordb_fwrite(ptr->fileid,&tmp,1);
    }
}

void Filell_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	Filell_t *ptr = Param;

	if((address+len) < ptr->Total_Size)
	{
        if(nordb_fseek(ptr->fileid, address, SEEK_SET)!=0)
                return;
        nordb_fwrite(ptr->fileid,data,len);
	}
}

void Filell_ReadBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	Filell_t *ptr = Param;

	if((address+len) < ptr->Total_Size)
	{
        if(nordb_fseek(ptr->fileid, address, SEEK_SET)!=0)
            return;
        nordb_fread(ptr->fileid,data,len);
	}
}

uint8_t Filell_flashCheck(void*Param)
{
	Filell_t *ptr = Param;
    return (ptr!=NULL && ptr->fileid!=NULL) ? 1:0;
}

const char *Filell_DriverName(void*Param)
{
	static const char DName[] = "FileDB";
	return DName;
}

NorDB_HWLayer *Filell_Init(char *path,uint16_t SectorSize,uint16_t TotalSector)
{
    NorDB_HWLayer *Filehw = nordb_malloc(sizeof(NorDB_HWLayer));
	if(Filehw==NULL)	return NULL;

	uint32_t total_Size = TotalSector *  SectorSize;
	Filell_t *Filesw = nordb_malloc(sizeof(Filell_t));
	if(Filesw==NULL)
	{
		nordb_free(Filesw);
		return NULL;
	}

    nordb_memset(Filesw,0,sizeof(Filell_t));
	Filesw->Sector_Size = SectorSize;
	Filesw->Total_Sector = TotalSector;
	Filesw->Total_Size	= total_Size;
	Filesw->fileid = nordb_fopen(path,"r+b");
	if(Filesw->fileid==NULL)
	{
		nordb_free(Filesw);
		nordb_free(Filehw);
		return NULL;
	}

    /*set filesize*/
    if(nordb_fseek(Filesw->fileid, total_Size, SEEK_SET)!=0)
    {
        Filell_Del(Filehw);
		return NULL;
    }
    nordb_fflush(Filesw->fileid);

	/*init sema*/
	if(!NorDB_sem_init(&Filehw->sema))
	{
		Filell_Del(Filehw);
		return NULL;
	}

	Filehw->SectorSize 		= SectorSize;
	Filehw->SectorNumber	= TotalSector;
	Filehw->Param			= Filesw;
	Filehw->Erase 			= Filell_Erase;
	Filehw->SectorErace		= Filell_SectorErace;
	Filehw->WriteBuffer		= Filell_WriteBuffer;
	Filehw->ReadBuffer		= Filell_ReadBuffer;
	Filehw->DriverCheck		= Filell_flashCheck;
	Filehw->DriverName		= Filell_DriverName;

	return Filehw;
}


void Filell_Del(NorDB_HWLayer *fdb)
{
    if(fdb->Param)
    {
        Filell_t *ptr = (Filell_t*)fdb->Param;
        if( ptr->fileid!=NULL )
        {
            nordb_fflush(ptr->fileid);
            nordb_fclose(ptr->fileid);
        }
        nordb_free(fdb->Param);
    }
    nordb_free(fdb);
}