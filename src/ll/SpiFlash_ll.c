
#include"ll/SpiFlash_ll.h"

SF_t SF;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SF_WaitForWriteEnd(SpiFlashll_HW *SpiHw)
{
    //usleep(1000);
    SpiHw->SpiChipSelect(0,true);
    uint8_t wait_arg = 0x05;
    SpiHw->SpiWrite(&wait_arg, 1);
    
    uint8_t status_reg_send = 0xA5;
    uint8_t status_reg_recive = 0xff;
    do
    {
        SpiHw->SpiStream(&status_reg_send ,&status_reg_recive, 1);
        SF.StatusRegister1 = status_reg_recive;
        //usleep(1000);
        
    }while( (SF.StatusRegister1 & 0x01) == 0x01);
    SpiHw->SpiChipSelect(1, true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SF_WriteEnable(SpiFlashll_HW *SpiHw)
{
    SpiHw->SpiChipSelect(0,true);
    uint8_t write_en = 0x06;
    SpiHw->SpiWrite(&write_en, 1);
    SpiHw->SpiChipSelect(1,true);
    //usleep(1000);
}

void SF_Writedisable(SpiFlashll_HW *SpiHw)
{
    SpiHw->SpiChipSelect(0,true);
    uint8_t write_en = 0x04;
    SpiHw->SpiWrite(&write_en, 1);
    SpiHw->SpiChipSelect(1,true);
    //usleep(1000);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SP_Read_Manifacture_Device_ID(SpiFlashll_HW *SpiHw)
{
    uint32_t Temp = 0;
    uint8_t  Temp0 = 0, Temp1 = 0, Temp2 = 0;

    SpiHw->SpiChipSelect(0, true);

    uint8_t send = 0x9F;
    SpiHw->SpiWrite(&send, 1);

    uint8_t dummy_byte = 0xA5;
    SpiHw->SpiStream(&Temp0, &dummy_byte, 1);
    SpiHw->SpiStream(&Temp1, &dummy_byte, 1);
    SpiHw->SpiStream(&Temp2, &dummy_byte, 1);
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

    SpiHw->SpiChipSelect(1, true);

    printf("devic id: %u \n", Temp);

    SF.PageSize = 256;
	SF.SectorSize = 0x1000;
    SF.BlockCount = 128;
	SF.SectorCount = SF.BlockCount * 16;
	SF.PageCount = (SF.SectorCount * SF.SectorSize) / SF.PageSize;
	SF.BlockSize = SF.SectorSize * 16;
	SF.CapacityInKiloByte = (SF.SectorCount * SF.SectorSize) / 1024;

    printf("SF Page Size: %d Bytes\r\n", SF.PageSize);
	printf("SF Page Count: %d\r\n", SF.PageCount);
	printf("SF Sector Size: %d Bytes\r\n", SF.SectorSize);
	printf("SF Sector Count: %d\r\n", SF.SectorCount);
	printf("SF Block Size: %d Bytes\r\n", SF.BlockSize);
	printf("SF Block Count: %d\r\n", SF.BlockCount);
	printf("SF Capacity: %d KiloBytes\r\n", SF.CapacityInKiloByte);
	printf("SF Init Done\r\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SF_EraseChip(SpiFlashll_HW *SpiHw)
{
    // SF_WaitForWriteEnd();
	SF_WriteEnable(SpiHw);
    SpiHw->SpiChipSelect(0,true);

    uint8_t write = 0xc7;
    SpiHw->SpiWrite(&write, 1);

    SpiHw->SpiChipSelect(1, true);
    SF_WaitForWriteEnd(SpiHw);
	SF_Writedisable(SpiHw);
}

//*****************************************************************
void SF_EraseSector(SpiFlashll_HW *SpiHw, uint32_t SectorAddress)
{
    // SF_WaitForWriteEnd();
    //uint32_t SectorAddress = SectorNumber*4096;
	SF_WriteEnable(SpiHw);
    SpiHw->SpiChipSelect(0,true);

    uint8_t write = 0x20;
    SpiHw->SpiWrite(&write, 1);
    write = (SectorAddress & 0xFF0000) >> 16;
    SpiHw->SpiWrite(&write, 1);
    write = (SectorAddress & 0xFF00) >> 8;
    SpiHw->SpiWrite(&write, 1);
    write = (SectorAddress & 0xFF);
    SpiHw->SpiWrite(&write, 1);

    SpiHw->SpiChipSelect(1, true);
    SF_WaitForWriteEnd(SpiHw);
	SF_Writedisable(SpiHw);
}
//*****************************************************************

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SF_WriteByte(SpiFlashll_HW *SpiHw, uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	SF_WriteEnable(SpiHw);
    SpiHw->SpiChipSelect(0,true);

    uint8_t buff[10];
    uint32_t index = 0;

    uint8_t write_arg = 0x02;
    buff[index++] = 0x02;

    buff[index++] = ((WriteAddr_inBytes & 0xff0000) >> 16);
    buff[index++] = ((WriteAddr_inBytes & 0xff00) >> 8);
    buff[index++] = ((WriteAddr_inBytes & 0xff));

    buff[index++]= pBuffer;
    
    SpiHw->SpiWrite(buff,index);
    
    SpiHw->SpiChipSelect(1, true);
    SF_WaitForWriteEnd(SpiHw);
    SF_Writedisable(SpiHw);
}

void SF_WriteMoreByte(SpiFlashll_HW *SpiHw, uint8_t *send, uint32_t WriteAddr_inBytes, uint32_t len)
{
    SF_WriteEnable(SpiHw);
    SpiHw->SpiChipSelect(0,true);

    uint8_t buff[len+4];
    uint32_t index = 0;

    uint8_t write_arg = 0x02;
    buff[index++] = 0x02;

    buff[index++] = ((WriteAddr_inBytes & 0xff0000) >> 16);
    buff[index++] = ((WriteAddr_inBytes & 0xff00) >> 8);
    buff[index++] = ((WriteAddr_inBytes & 0xff));

    for(int i=0; i<len; i++)
        buff[index++]= send[i];
    
    SpiHw->SpiWrite(buff,index);
    
    SpiHw->SpiChipSelect(1, true);
    SF_WaitForWriteEnd(SpiHw);
    SF_Writedisable(SpiHw);
}

void SF_Write(SpiFlashll_HW *SpiHw, uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes)                      //my write func
{                                   
    int test1 = (WriteAddr_inBytes >> 8);
    int test2 = ((WriteAddr_inBytes + len) >> 8) ;

    int end_of_first_page = (WriteAddr_inBytes & 0x7FFF00) + 0xFF;
    int len_in_first_page = end_of_first_page - WriteAddr_inBytes+1;
    int num_of_full_page = (len - len_in_first_page)/256;
    int len_in_last_page = len - ( num_of_full_page*256 + len_in_first_page);
    int Addr_of_next_page;

    if (test1 == test2)
    {
        SF_WriteMoreByte(SpiHw, send, WriteAddr_inBytes, len);
    }
    else
    {
        SF_WriteMoreByte(SpiHw, send, WriteAddr_inBytes, len_in_first_page);
        Addr_of_next_page = end_of_first_page + 1;
        send += len_in_first_page;

        for( ; 0<num_of_full_page; num_of_full_page--)
        {
            // for(int i=0; i<10; i++)
            //     printf("%c", send[i]);
            // printf("\n");

            SF_WriteMoreByte(SpiHw, send,  Addr_of_next_page, 256);
            Addr_of_next_page += 256;
            send += 256;

        }

        if( len_in_last_page != 0)
            SF_WriteMoreByte(SpiHw, send, Addr_of_next_page, len_in_last_page);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SF_ReadByte(SpiFlashll_HW *SpiHw, uint8_t *pBuffer, uint32_t ReadAddr)
{
    // SF_WaitForWriteEnd();
    SpiHw->SpiChipSelect(0, true);

    uint8_t buff[10];
    uint8_t index = 0;
    uint8_t read_arg = 0x0B;
    uint8_t zero = 0;
    uint8_t dummy = 0XA5;


    buff[index++] = read_arg;
    buff[index++] = ((ReadAddr & 0xff0000) >> 16);
    buff[index++] = ((ReadAddr & 0xff00) >> 8);
    buff[index++] = ((ReadAddr & 0xff));
    buff[index++] = zero;
        
    SpiHw->SpiWrite(buff, index);
    SpiHw->SpiStream(&dummy, pBuffer, 1);

    SpiHw->SpiChipSelect(1, true);
    SF_WaitForWriteEnd(SpiHw);
    // printf("data in flash is: \n");
}

void SF_ReadMoreByte(SpiFlashll_HW *SpiHw, uint8_t *recive, uint32_t len, uint32_t ReadAddr_inBytes)
{
    // SF_WaitForWriteEnd();
    SpiHw->SpiChipSelect(0, true);

    uint8_t buff[10];
    uint8_t index = 0;
    uint8_t read_arg = 0x0B;
    uint8_t zero = 0;
    uint8_t dummy[len];

    memset(dummy, 0xA5, len*(sizeof(uint8_t)) );

    buff[index++] = read_arg;
    buff[index++] = ((ReadAddr_inBytes & 0xff0000) >> 16);
    buff[index++] = ((ReadAddr_inBytes & 0xff00) >> 8);
    buff[index++] = ((ReadAddr_inBytes & 0xff));
    buff[index++] = zero;
        
    SpiHw->SpiWrite(buff, index);
    SpiHw->SpiStream(dummy, recive, len);

    SpiHw->SpiChipSelect(1, true);
    SF_WaitForWriteEnd(SpiHw);
    // printf("data in flash is: \n");
}

void SF_Read(SpiFlashll_HW *SpiHw, uint8_t *recive, uint32_t len, uint32_t ReadAddr_inBytes)
{
    int test1 = (ReadAddr_inBytes >> 8);
    int test2 = ((ReadAddr_inBytes + len) >> 8) ;

    int end_of_first_page = (ReadAddr_inBytes & 0x7FFF00) + 0xFF;
    int len_in_first_page = end_of_first_page - ReadAddr_inBytes+1;
    int num_of_full_page = (len - len_in_first_page)/256;
    int len_in_last_page = len - ( num_of_full_page*256 + len_in_first_page);
    int Addr_of_next_page;

     if (test1 == test2)
    {
        SF_ReadMoreByte(SpiHw, recive, len, ReadAddr_inBytes);
    }
    else
    {
        SF_ReadMoreByte(SpiHw, recive, len_in_first_page, ReadAddr_inBytes);
        Addr_of_next_page = end_of_first_page + 1;
        recive += len_in_first_page;

        for( ; 0<num_of_full_page; num_of_full_page--)
        {
            SF_ReadMoreByte(SpiHw, recive, 256,  Addr_of_next_page);
            Addr_of_next_page += 256;
            recive += 256;

        }

        if( len_in_last_page != 0)
            SF_ReadMoreByte(SpiHw, recive, len_in_last_page, Addr_of_next_page);
    }
}


//**************************************************************************************

void SPIFlashll_Erase(void *Param)
{
	SpiFlashll_HW *ptr = Param;

	//nordb_memset(ptr->Buffer,0xFF,ptr->Total_Size);
    SF_EraseChip(ptr);
}

void SPIFlashll_SectorErase(void *Param, uint32_t address)
{
	SpiFlashll_HW *ptr = Param;

	//uint16_t Sector_Number  = address / ptr->Sector_Size;
    uint16_t Sector_Number  = address / ((SpiFlashll_t*)ptr->Param)->Sector_Size;
	if(Sector_Number < ((SpiFlashll_t*)ptr->Param)->Total_Sector)
		//nordb_memset(ptr->Buffer+(Sector_Number*ptr->Sector_Size),0xFF,ptr->Sector_Size);
        SF_EraseSector(ptr, address);
}

void SPIFlashll_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	SpiFlashll_HW *ptr = Param;

	if((address+len) < ((SpiFlashll_t*)ptr->Param)->Total_Size)
	{
		//nordb_memcpy(&ptr->Buffer[address],data,len);
        SF_Write(ptr, data, len, address);
		//printf("wRAM (%d)->%d\n",address,len);
	}
}

void SPIFlashll_ReadBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	SpiFlashll_HW *ptr = Param;

	if((address+len) < ((SpiFlashll_t*)ptr->Param)->Total_Size)
	{
		// nordb_memcpy(data,&ptr->Buffer[address],len);
        SF_Read(ptr, data, len, address);
		//printf("rRAM (%d)->%d\n",address,len);
	}
}

uint8_t SPIFlashll_flashCheck(void*Param)
{
	//RamBuffer *ram = (RamBuffer*)Param;
	return 1;
}

const char *SPIFlashll_DriverName(void*Param)
{
	static const char DName[] = "RamDB";
	return DName;
}


NorDB_HWLayer *SpiFlashll_Init(SpiFlashll_HW *SpiHw,uint16_t SectorSize,uint16_t TotalSector)
{
    NorDB_HWLayer *SpiFlashHw = nordb_malloc(sizeof(NorDB_HWLayer));
    memset(SpiFlashHw, 0, sizeof(NorDB_HWLayer));
	if(SpiFlashHw==NULL)	return NULL;
    if(SpiHw==NULL)	return NULL;

	uint32_t total_Size = TotalSector *  SectorSize;
	SpiFlashll_t *SpiFlashSw = nordb_malloc(sizeof(SpiFlashll_t));
    //SpiFlashSw = (SpiFlashll_t*)SpiHw->Param;
	if(SpiFlashSw==NULL)
	{
		nordb_free(SpiFlashSw);
		return NULL;
	}
    // SpiFlashHw->LastReadSector = 0;
    // SpiFlashHw->LastWriteSector = 0;
    // SpiFlashHw->Synced = false;
	SpiFlashSw->Sector_Size = SectorSize;
	SpiFlashSw->Total_Sector = TotalSector;
	SpiFlashSw->Total_Size	= total_Size;
	SpiFlashSw->Buffer = nordb_malloc(total_Size);

    //SpiHw->SpiChipSelect = 

	if(SpiFlashSw->Buffer==NULL)
	{
		nordb_free(SpiFlashSw);
		nordb_free(SpiFlashSw);
		return NULL;
	}

    // _SpiHw = *SpiHw;
    // SpiHw->SpiChipSelect = 

	/*init sema*/
	if(!NorDB_sem_init(&SpiFlashHw->sema))
	{
		SpiFlash_Del(SpiFlashHw);
		return NULL;
	}

	SpiFlashHw->SectorSize 		= SectorSize;
	SpiFlashHw->SectorNumber	= TotalSector;
	SpiFlashHw->Param			= SpiHw;
	SpiFlashHw->Erase 			= SPIFlashll_Erase;
	SpiFlashHw->SectorErace		= SPIFlashll_SectorErase;
	SpiFlashHw->WriteBuffer		= SPIFlashll_WriteBuffer;
	SpiFlashHw->ReadBuffer		= SPIFlashll_ReadBuffer;
	SpiFlashHw->DriverCheck		= SPIFlashll_flashCheck;
	SpiFlashHw->DriverName		= SPIFlashll_DriverName;

	return SpiFlashHw;
}

void SpiFlash_Del(NorDB_HWLayer *db)
{
	nordb_free((((SpiFlashll_HW*)db->Param)->Param->Buffer));
	nordb_free(db->Param);
	nordb_free(db);
}

//**************************************************************************************



