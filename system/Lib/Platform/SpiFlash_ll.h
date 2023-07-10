#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "../CH341/ch341.h"

//**********************************************************
#include "../NorDB/NorDB.h"

typedef struct
{
	uint16_t 		Sector_Size;
	uint16_t 		Total_Sector;
	uint32_t		Total_Size;
	uint8_t  		*Buffer;
}SpiFlashll_t;


NorDB_HWLayer *SpiFlashll_Init(uint16_t SectorSize,uint16_t TotalSector);
void SpiFlash_Del(NorDB_HWLayer *db);
//**********************************************************


typedef struct
{
	uint8_t UniqID[8];
	uint16_t PageSize;
	uint32_t PageCount;
	uint32_t 
	SectorSize;
	uint32_t SectorCount;
	uint32_t BlockSize;
	uint32_t BlockCount;
	uint32_t CapacityInKiloByte;
	uint8_t StatusRegister1;
	uint8_t StatusRegister2;
	uint8_t StatusRegister3;
	uint8_t Lock;

} w25qxx_t;

///////////////////////////////////////////////////////////////////////////
void W25qxx_WaitForWriteEnd(void);

///////////////////////////////////////////////////////////////////////////
void W25Q_Read_Manifacture_Device_ID(void);
bool W25qxx_Init(void);

///////////////////////////////////////////////////////////////////////////  
void W25qxx_EraseChip(void);
//void W25qxx_EraseSector(uint32_t SectorAddr);
//void W25qxx_EraseBlock(uint32_t BlockAddr);

void W25qxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes);
void W25qxx_WriteMoreByte(uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes);
void W25qxx_Write(uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes);
void W25qxx_Write2(uint8_t *ptr, uint32_t addr, int len);

void W25qxx_ReadByte(unsigned char *pBuffer, uint32_t ReadAddr);
void W25qxx_ReadMoreByte(uint8_t *recive, uint32_t len, uint32_t WriteAddr_inBytes);
void w25qxx_Read(uint8_t *recive, uint32_t len, uint32_t ReadAddr_inBytes);
void W25qxx_Read2(uint8_t *recive, uint32_t addr, int len);
void W25qxx_Read3(uint8_t *pBuffer, uint32_t ReadAddr, int len);