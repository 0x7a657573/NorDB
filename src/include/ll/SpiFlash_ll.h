#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// #include "../CH341/ch341.h"

//**********************************************************
#include "../NorDB.h"

typedef struct
{
	uint16_t 		Sector_Size;
	uint16_t 		Total_Sector;
	uint32_t		Total_Size;
	uint8_t  		*Buffer;
}SpiFlashll_t;

typedef struct
{
	SpiFlashll_t 	*Param;
	
	bool	(*SpiChipSelect)(unsigned int cs, bool enable);
	bool	(*SpiStream)(const unsigned char *in, unsigned char *out, unsigned int size);
	bool	(*SpiWrite)(const unsigned char *in, unsigned int size);
	bool	(*SpiRead)(unsigned char *out, unsigned int size);

}SpiFlashll_HW;

NorDB_HWLayer *SpiFlashll_Init(SpiFlashll_HW *SpiHw,uint16_t SectorSize,uint16_t TotalSector);
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

} SF_t;

///////////////////////////////////////////////////////////////////////////
void SF_WaitForWriteEnd(SpiFlashll_HW *SpiHw);

///////////////////////////////////////////////////////////////////////////
void SFQ_Read_Manifacture_Device_ID(SpiFlashll_HW *SpiHw);
bool SF_Init(SpiFlashll_HW *SpiHw);

///////////////////////////////////////////////////////////////////////////  
void SF_EraseChip(SpiFlashll_HW *SpiHw);
//void SF_EraseSector(uint32_t SectorAddr);
//void SF_EraseBlock(uint32_t BlockAddr);

void SF_WriteByte(SpiFlashll_HW *SpiHw, uint8_t pBuffer, uint32_t WriteAddr_inBytes);
void SF_WriteMoreByte(SpiFlashll_HW *SpiHw, uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes);
void SF_Write(SpiFlashll_HW *SpiHw, uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes);

void SF_ReadByte(SpiFlashll_HW *SpiHw, unsigned char *pBuffer, uint32_t ReadAddr);
void SF_ReadMoreByte(SpiFlashll_HW *SpiHw, uint8_t *recive, uint32_t len, uint32_t WriteAddr_inBytes);
void SF_Read(SpiFlashll_HW *SpiHw, uint8_t *recive, uint32_t len, uint32_t ReadAddr_inBytes);
