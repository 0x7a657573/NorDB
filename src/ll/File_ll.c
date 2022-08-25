///*
// * FileDB.c
// *
// *  Created on: Feb 16, 2019
// *      Author: Mazarei
// */
//#include "Object.h"
//#include <ql_fs.h>
//#include <DebugTask.h>
//#include <ll/File_ll.h>
//#include <ql_error.h>
//
//#define ExDebug(...)		//debugf(Window31,__VA_ARGS__)
//#define debug(...)      	//debugf(Window31,__VA_ARGS__)
//#define Errdebug(...)		debugf(Window24,__VA_ARGS__)
//#define Infodebug(...)      debugf(Window33,__VA_ARGS__)
//#define portMAX_DELAY		0xFFFFFFFF
//
//static SemaphoreHandle_t		xSemaphore = 0;
//#define xSemaphoreTake(x,y)		Ql_OS_TakeMutex(x)
//#define xSemaphoreGive(x)		Ql_OS_GiveMutex(x)
//
//void EXFile_Erase(void*Param);
//void EXFile_SectorErace(void*Param,uint32_t address);
//void EXFile_WriteByte(void*Param,uint32_t address,uint8_t data);
//uint8_t EXFile_ReadByte(void*Param,uint32_t address);
//void EXFile_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len);
//void EXFile_ReadBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len);
//uint8_t EXFile_flashCheck(void*Param);
//const char *EXFile_DriverName(void*Param);
//
//void EXFile_Erase(void*Param)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
////	FDB->FileHandle = Ql_FS_Open(DBFileName, QL_FS_CREATE | QL_FS_READ_WRITE);
////	FDB->Enable = (FDB->FileHandle<0) ? false:true;
//	xSemaphoreTake( xSemaphore, portMAX_DELAY );
//	if(FDB->Enable)
//	{
//		Ql_memset(FDB->Sector,0xFF,DBSectorSize);
//		if(Ql_FS_Seek(FDB->FileHandle, 0 ,QL_FS_FILE_BEGIN)==QL_RET_OK)
//		{
//
//			for(uint32_t i=FDB->LightDB->StartOffset;i<(FDB->LightDB->StartOffset+FDB->LightDB->Total_Size);i+=FDB->LightDB->Sector_Size)
//			{
//				u32 W;
//				s32 ret = Ql_FS_Write(FDB->FileHandle, FDB->Sector, DBSectorSize, &W);
//				if(ret!=QL_RET_OK || W!=DBSectorSize)
//				{
//					Errdebug("File_Erase Error %u",ret);
//					FDB->Enable = false;
//					break;
//				}
//			}
//			Ql_FS_Flush(FDB->FileHandle);
//			debug("FDB Erase OK.");
//		}
////		Ql_FS_Close(FDB->FileHandle);
////		FDB->FileHandle = -1;
//	}
//	xSemaphoreGive( xSemaphore );
//}
//
//void EXFile_SectorErace(void*Param,uint32_t address)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
////	FDB->FileHandle = Ql_FS_Open(DBFileName, QL_FS_CREATE | QL_FS_READ_WRITE);
////	FDB->Enable = (FDB->FileHandle<0) ? false:true;
//	xSemaphoreTake( xSemaphore, portMAX_DELAY );
//	if(FDB->Enable)
//	{
//		Ql_memset(FDB->Sector,0xFF,DBSectorSize);
//		if(Ql_FS_Seek(FDB->FileHandle, address ,QL_FS_FILE_BEGIN)==QL_RET_OK)
//		{
//			u32 W;
//			s32 ret = Ql_FS_Write(FDB->FileHandle, FDB->Sector, DBSectorSize, &W);
//			if(ret!=QL_RET_OK || W!=DBSectorSize)
//			{
//				Errdebug("File_SectorErace Error %u",ret);
//				FDB->Enable = false;
//			}
//		}
//		Ql_FS_Flush(FDB->FileHandle);
//		debug("FDB Sector %u Erase OK.",address/DBSectorSize);
////		Ql_FS_Close(FDB->FileHandle);
////		FDB->FileHandle = -1;
//	}
//	xSemaphoreGive( xSemaphore );
//}
//
//void EXFile_WriteByte(void*Param,uint32_t address,uint8_t data)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
//	uint8_t Byte = EXFile_ReadByte(Param,address);
////	FDB->FileHandle = Ql_FS_Open(DBFileName, QL_FS_CREATE | QL_FS_READ_WRITE);
////	FDB->Enable = (FDB->FileHandle<0) ? false:true;
//	xSemaphoreTake( xSemaphore, portMAX_DELAY );
//	if(FDB->Enable)
//	{
//		if(Ql_FS_Seek(FDB->FileHandle, address ,QL_FS_FILE_BEGIN)==QL_RET_OK)
//		{
//			data = data & Byte;
//			debug("Write Byte To : %u-%X",address,data);
//			u32 W;
//			s32 ret = Ql_FS_Write(FDB->FileHandle, &data, 1, &W);
//			if(ret!=QL_RET_OK || W!=1)
//			{
//				Errdebug("File_WriteByte Error %u",ret);
//				FDB->Enable = false;
//			}
//			else
//				Ql_FS_Flush(FDB->FileHandle);
//		}
////		Ql_FS_Close(FDB->FileHandle);
////		FDB->FileHandle = -1;
//	}
//	xSemaphoreGive( xSemaphore );
//}
//
//void EXFile_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
////	FDB->FileHandle = Ql_FS_Open(DBFileName, QL_FS_CREATE | QL_FS_READ_WRITE);
////	FDB->Enable = (FDB->FileHandle<0) ? false:true;
//	xSemaphoreTake( xSemaphore, portMAX_DELAY );
//	if(FDB->Enable)
//	{
//
//		if(Ql_FS_Seek(FDB->FileHandle, address ,QL_FS_FILE_BEGIN)==QL_RET_OK)
//		{
//			ExDebug("Write Bytes To : %u-%u",address,len);
//			u32 W;
//			s32 ret = Ql_FS_Write(FDB->FileHandle, data, len, &W);
//			if(ret!=QL_RET_OK || W!=len)
//			{
//				Errdebug("File_WriteBuffer Error %u",ret);
//				FDB->Enable = false;
//			}
//			else
//				Ql_FS_Flush(FDB->FileHandle);
//		}
////		Ql_FS_Close(FDB->FileHandle);
////		FDB->FileHandle = -1;
//	}
//	xSemaphoreGive( xSemaphore );
//}
//
//uint8_t EXFile_ReadByte(void*Param,uint32_t address)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
////	FDB->FileHandle = Ql_FS_Open(DBFileName, QL_FS_CREATE | QL_FS_READ_WRITE);
////	FDB->Enable = (FDB->FileHandle<0) ? false:true;
//	xSemaphoreTake( xSemaphore, portMAX_DELAY );
//	if(FDB->Enable)
//	{
//		if(Ql_FS_Seek(FDB->FileHandle, address ,QL_FS_FILE_BEGIN)==QL_RET_OK)
//		{
//			u32 Rb;
//			s32 ret = Ql_FS_Read(FDB->FileHandle,FDB->Sector,1,&Rb);
//			if(ret!=QL_RET_OK || Rb!=1)
//			{
//				Errdebug("File_ReadByte Error %u",ret);
//				FDB->Enable = false;
//			}
//			ExDebug("Read Byte From : %u-%X",address,FDB->Sector[0]);
//		}
////		Ql_FS_Close(FDB->FileHandle);
////		FDB->FileHandle = -1;
//	}
//	uint8_t RetVal = FDB->Sector[0];
//	xSemaphoreGive( xSemaphore );
//	return RetVal;
//}
//
//void EXFile_ReadBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
////	FDB->FileHandle = Ql_FS_Open(DBFileName, QL_FS_CREATE | QL_FS_READ_WRITE);
////	FDB->Enable = (FDB->FileHandle<0) ? false:true;
//	xSemaphoreTake( xSemaphore, portMAX_DELAY );
//	if(FDB->Enable)
//	{
//		if(Ql_FS_Seek(FDB->FileHandle, address ,QL_FS_FILE_BEGIN)==QL_RET_OK)
//		{
//			ExDebug("Read Bytes From : %u-%u",address,len);
//			u32 Rb;
//			s32 ret = Ql_FS_Read(FDB->FileHandle,data,len,&Rb);
//			if(ret!=QL_RET_OK || Rb!=len)
//			{
//				Errdebug("File_ReadBuffer Error %u",ret);
//				FDB->Enable = false;
//			}
//		}
////		Ql_FS_Close(FDB->FileHandle);
////		FDB->FileHandle = -1;
//	}
//	xSemaphoreGive( xSemaphore );
//}
//
//uint8_t EXFile_flashCheck(void*Param)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
//	xSemaphoreTake( xSemaphore, portMAX_DELAY );
//	uint8_t retval = FDB->Enable;
//	xSemaphoreGive( xSemaphore );
//	return retval;
//}
//
//const char *EXFile_DriverName(void*Param)
//{
//	FileDB_Param *FDB =(FileDB_Param*) Param;
//	static const char DName[] = "iFlash";
//	return DName;
//}
//
//LightDB_HWLayer *FileDB_Init(uint16_t RecordSize,uint16_t TotalSector)
//{
//
//	LightDB_HWLayer *Ramhw = new(LightDB_HWLayer);
//	if(Ramhw==NULL)	return NULL;
//
//	FileDB_Param *FDb = new(FileDB_Param);
//	if(FDb==NULL)
//	{
//		delete(Ramhw);
//		return NULL;
//	}
//
//	s32 Size = Ql_FS_GetSize(DBFileName);
//	if(Size<0)	Size = 0;
//	debug("DB File Size: %u",Size);
//
//	FDb->LightDB = Ramhw;
//	xSemaphore = Ql_OS_CreateMutex("FDB");
//	if(xSemaphore == 0)
//	{
//		delete(Ramhw);
//		delete(FDb);
//		return NULL;
//	}
//	debug("Mutex :%u",xSemaphore);
//
//	FDb->FileHandle = Ql_FS_Open(DBFileName, QL_FS_CREATE | QL_FS_READ_WRITE);
//	if(FDb->FileHandle<0)	/*If Error In Open File*/
//	{
//		Errdebug("Can Not Make Point.db %u",FDb->FileHandle);
//		delete(FDb->LightDB);
//		delete(FDb);
//		return NULL;
//	}
////	Ql_FS_Close(FDb->FileHandle);
////	FDb->FileHandle = -1;
//	FDb->Enable = true;
//
//
//	debug("File DB Init Ok. %u",FDb->FileHandle);
//
//	Ramhw->HWUseSelect 	   = NULL;
//
//	Ramhw->TotalStorRecord = -1;
//	Ramhw->Refresh_Time    = 0;
//	Ramhw->HWParamiter = (FileDB_Param*)FDb;
//	Ramhw->Sector_Size = DBSectorSize;
//	Ramhw->StartOffset = 0;
//	Ramhw->Total_Size  = DBSectorSize * TotalSector;
//	Ramhw->Record_Size  = RecordSize;
//
//	Ramhw->Erase 			= EXFile_Erase;
//	Ramhw->SectorErace		= EXFile_SectorErace;
//	Ramhw->WriteByte		= EXFile_WriteByte;
//	Ramhw->WriteBuffer		= EXFile_WriteBuffer;
//	Ramhw->ReadByte			= EXFile_ReadByte;
//	Ramhw->ReadBuffer		= EXFile_ReadBuffer;
//	Ramhw->flashCheck		= EXFile_flashCheck;
//	Ramhw->DriverName		= EXFile_DriverName;
//
//	if(Size < (DBSectorSize * TotalSector))
//	{
//		debug("Erase FileDB");
//		EXFile_Erase(FDb);
//	}
//
////	EXFile_WriteByte(FDb,0,0xAA);
////	uint8_t Read = EXFile_ReadByte(FDb,0);
////	debug("Read : %X",Read);
//	return Ramhw;
//}
