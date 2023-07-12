
#include"ll/SpiFlash_ll.h"

w25qxx_t w25qxx;
SpiFlashll_HW _SpiHw;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25qxx_WaitForWriteEnd(void)
{
    usleep(1000);
    _SpiHw.SpiChipSelect(0,true);
    uint8_t wait_arg = 0x05;
    _SpiHw.SpiWrite(&wait_arg, 1);
    
    uint8_t status_reg_send = 0xA5;
    uint8_t status_reg_recive = 0xff;
    do
    {
        _SpiHw.SpiStream( &status_reg_send ,&status_reg_recive, 1);
        w25qxx.StatusRegister1 = status_reg_recive;
        usleep(1000);
        
    }while( (w25qxx.StatusRegister1 & 0x01) == 0x01);
    _SpiHw.SpiChipSelect(1, true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25qxx_WriteEnable()
{
    _SpiHw.SpiChipSelect(0,true);
    uint8_t write_en = 0x06;
    _SpiHw.SpiWrite( &write_en, 1);
    _SpiHw.SpiChipSelect(1,true);
    usleep(1000);
}

void W25qxx_Writedisable()
{
    _SpiHw.SpiChipSelect(0,true);
    uint8_t write_en = 0x04;
    _SpiHw.SpiWrite( &write_en, 1);
    _SpiHw.SpiChipSelect(1,true);
    usleep(1000);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25Q_Read_Manifacture_Device_ID(void)
{
    uint32_t Temp = 0;
    uint8_t  Temp0 = 0, Temp1 = 0, Temp2 = 0;

    _SpiHw.SpiChipSelect(0, true);

    uint8_t send = 0x9F;
    _SpiHw.SpiWrite( &send, 1);

    uint8_t dummy_byte = 0xA5;
    _SpiHw.SpiStream( &Temp0, &dummy_byte, 1);
    _SpiHw.SpiStream( &Temp1, &dummy_byte, 1);
    _SpiHw.SpiStream( &Temp2, &dummy_byte, 1);
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

    _SpiHw.SpiChipSelect(1, true);

    printf("devic id: %u \n", Temp);

    w25qxx.PageSize = 256;
	w25qxx.SectorSize = 0x1000;
    w25qxx.BlockCount = 128;
	w25qxx.SectorCount = w25qxx.BlockCount * 16;
	w25qxx.PageCount = (w25qxx.SectorCount * w25qxx.SectorSize) / w25qxx.PageSize;
	w25qxx.BlockSize = w25qxx.SectorSize * 16;
	w25qxx.CapacityInKiloByte = (w25qxx.SectorCount * w25qxx.SectorSize) / 1024;

    printf("w25qxx Page Size: %d Bytes\r\n", w25qxx.PageSize);
	printf("w25qxx Page Count: %d\r\n", w25qxx.PageCount);
	printf("w25qxx Sector Size: %d Bytes\r\n", w25qxx.SectorSize);
	printf("w25qxx Sector Count: %d\r\n", w25qxx.SectorCount);
	printf("w25qxx Block Size: %d Bytes\r\n", w25qxx.BlockSize);
	printf("w25qxx Block Count: %d\r\n", w25qxx.BlockCount);
	printf("w25qxx Capacity: %d KiloBytes\r\n", w25qxx.CapacityInKiloByte);
	printf("w25qxx Init Done\r\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25qxx_EraseChip(void)
{
    // W25qxx_WaitForWriteEnd();
	W25qxx_WriteEnable();
    _SpiHw.SpiChipSelect(0,true);

    uint8_t write = 0xc7;
    _SpiHw.SpiWrite( &write, 1);

    _SpiHw.SpiChipSelect(1, true);
    W25qxx_WaitForWriteEnd();
	W25qxx_Writedisable();
}

//*****************************************************************
void W25qxx_EraseSector(uint32_t SectorAddress)
{
    // W25qxx_WaitForWriteEnd();
    //uint32_t SectorAddress = SectorNumber*4096;
	W25qxx_WriteEnable();
    _SpiHw.SpiChipSelect(0,true);

    uint8_t write = 0x20;
    _SpiHw.SpiWrite( &write, 1);
    write = (SectorAddress & 0xFF0000) >> 16;
    _SpiHw.SpiWrite(&write, 1);
    write = (SectorAddress & 0xFF00) >> 8;
    _SpiHw.SpiWrite(&write, 1);
    write = (SectorAddress & 0xFF);
    _SpiHw.SpiWrite(&write, 1);

    _SpiHw.SpiChipSelect(1, true);
    W25qxx_WaitForWriteEnd();
	W25qxx_Writedisable();
}
//*****************************************************************

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25qxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	W25qxx_WriteEnable();
    _SpiHw.SpiChipSelect(0,true);

    uint8_t buff[10];
    uint32_t index = 0;

    uint8_t write_arg = 0x02;
    buff[index++] = 0x02;

    buff[index++] = ((WriteAddr_inBytes & 0xff0000) >> 16);
    buff[index++] = ((WriteAddr_inBytes & 0xff00) >> 8);
    buff[index++] = ((WriteAddr_inBytes & 0xff));

    buff[index++]= pBuffer;
    
    _SpiHw.SpiWrite(buff,index);
    
    _SpiHw.SpiChipSelect(1, true);
    W25qxx_WaitForWriteEnd();
    W25qxx_Writedisable();
}

void W25qxx_WriteMoreByte(uint8_t *send, uint32_t WriteAddr_inBytes, uint32_t len)
{
    W25qxx_WriteEnable();
    _SpiHw.SpiChipSelect(0,true);

    uint8_t buff[len+4];
    uint32_t index = 0;

    uint8_t write_arg = 0x02;
    buff[index++] = 0x02;

    buff[index++] = ((WriteAddr_inBytes & 0xff0000) >> 16);
    buff[index++] = ((WriteAddr_inBytes & 0xff00) >> 8);
    buff[index++] = ((WriteAddr_inBytes & 0xff));

    for(int i=0; i<len; i++)
        buff[index++]= send[i];
    
    _SpiHw.SpiWrite(buff,index);
    
    _SpiHw.SpiChipSelect(1, true);
    W25qxx_WaitForWriteEnd();
    W25qxx_Writedisable();
}

void W25qxx_Write(uint8_t *send, uint32_t len, uint32_t WriteAddr_inBytes)                      //my write func
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
        W25qxx_WriteMoreByte(send, WriteAddr_inBytes, len);
    }
    else
    {
        W25qxx_WriteMoreByte(send, WriteAddr_inBytes, len_in_first_page);
        Addr_of_next_page = end_of_first_page + 1;
        send += len_in_first_page;

        for( ; 0<num_of_full_page; num_of_full_page--)
        {
            // for(int i=0; i<10; i++)
            //     printf("%c", send[i]);
            // printf("\n");

            W25qxx_WriteMoreByte(send,  Addr_of_next_page, 256);
            Addr_of_next_page += 256;
            send += 256;

        }

        if( len_in_last_page != 0)
            W25qxx_WriteMoreByte(send, Addr_of_next_page, len_in_last_page);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void W25qxx_ReadByte(uint8_t *pBuffer, uint32_t ReadAddr)
{
    // W25qxx_WaitForWriteEnd();
    _SpiHw.SpiChipSelect(0, true);

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
        
    _SpiHw.SpiWrite(buff, index);
    _SpiHw.SpiStream(&dummy, pBuffer, 1);

    _SpiHw.SpiChipSelect(1, true);
    W25qxx_WaitForWriteEnd();
    // printf("data in flash is: \n");
}

void W25qxx_ReadMoreByte(uint8_t *recive, uint32_t len, uint32_t ReadAddr_inBytes)
{
    // W25qxx_WaitForWriteEnd();
    _SpiHw.SpiChipSelect(0, true);

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
        
    _SpiHw.SpiWrite(buff, index);
    _SpiHw.SpiStream(dummy, recive, len);

    _SpiHw.SpiChipSelect(1, true);
    W25qxx_WaitForWriteEnd();
    // printf("data in flash is: \n");
}

void w25qxx_Read(uint8_t *recive, uint32_t len, uint32_t ReadAddr_inBytes)
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
        W25qxx_ReadMoreByte(recive, len, ReadAddr_inBytes);
    }
    else
    {
        W25qxx_ReadMoreByte(recive, len_in_first_page, ReadAddr_inBytes);
        Addr_of_next_page = end_of_first_page + 1;
        recive += len_in_first_page;

        for( ; 0<num_of_full_page; num_of_full_page--)
        {
            W25qxx_ReadMoreByte(recive, 256,  Addr_of_next_page);
            Addr_of_next_page += 256;
            recive += 256;

        }

        if( len_in_last_page != 0)
            W25qxx_ReadMoreByte(recive, len_in_last_page, Addr_of_next_page);
    }
}


//**************************************************************************************

void SPIFlashll_Erase(void *Param)
{
	SpiFlashll_t *ptr = Param;

	//nordb_memset(ptr->Buffer,0xFF,ptr->Total_Size);
    W25qxx_EraseChip();
}

void SPIFlashll_SectorErase(void*Param,uint32_t address)
{
	SpiFlashll_t *ptr = Param;

	uint16_t Sector_Number  = address / ptr->Sector_Size;
	if(Sector_Number < ptr->Total_Sector)
		//nordb_memset(ptr->Buffer+(Sector_Number*ptr->Sector_Size),0xFF,ptr->Sector_Size);
        W25qxx_EraseSector(address);
}

void SPIFlashll_WriteBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	SpiFlashll_t *ptr = Param;

	if((address+len) < ptr->Total_Size)
	{
		//nordb_memcpy(&ptr->Buffer[address],data,len);
        W25qxx_Write(data, len, address);
		//printf("wRAM (%d)->%d\n",address,len);
	}
}

void SPIFlashll_ReadBuffer(void*Param,uint32_t address,uint8_t *data,uint16_t len)
{
	SpiFlashll_t *ptr = Param;

	if((address+len) < ptr->Total_Size)
	{
		// nordb_memcpy(data,&ptr->Buffer[address],len);
        w25qxx_Read(data, len, address);
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

	if(SpiFlashSw->Buffer==NULL)
	{
		nordb_free(SpiFlashSw);
		nordb_free(SpiFlashSw);
		return NULL;
	}

    _SpiHw = *SpiHw;

	/*init sema*/
	if(!NorDB_sem_init(&SpiFlashHw->sema))
	{
		SpiFlash_Del(SpiFlashHw);
		return NULL;
	}

	SpiFlashHw->SectorSize 		= SectorSize;
	SpiFlashHw->SectorNumber	= TotalSector;
	SpiFlashHw->Param			= SpiFlashSw;
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
	nordb_free((((SpiFlashll_t*)db->Param)->Buffer));
	nordb_free(db->Param);
	nordb_free(db);
}

//**************************************************************************************



