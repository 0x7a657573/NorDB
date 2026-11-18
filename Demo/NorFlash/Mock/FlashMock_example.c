#include <stdio.h>
#include <stdlib.h>

#include <NorDB.h>
#include <ll/SerialNorFlash_ll.h>
#include "TestRoutines.h"
#include "SpiFlashMock.h"

#define FLASH_MOCK_MAX_BUSY_POLLS 16

/* Real W25Q64 SFDP dump: header, BFPT at 0x80, 16 DWORDs. */
static const uint8_t w25q64_sfdp[256] = {
	0x53,0x46,0x44,0x50,0x05,0x01,0x00,0xFF,0x00,0x05,0x01,0x10,0x80,0x00,0x00,0xFF,
	[0x10 ... 0x7F]=0xFF,
	[0x80]=0xE5,0x20,0xF9,0xFF,0xFF,0xFF,0xFF,0x03,0x44,0xEB,0x08,0x6B,0x08,0x3B,0x42,0xBB,
	0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0xFF,0x40,0xEB,0x0C,0x20,0x0F,0x52,
	0x10,0xD8,0x00,0x00,0x36,0x02,0xA6,0x00,0x82,0xEA,0x14,0xC4,0xE9,0x63,0x76,0x33,
	0x7A,0x75,0x7A,0x75,0xF7,0xA2,0xD5,0x5C,0x19,0xF7,0x4D,0xFF,0xE9,0x30,0xF8,0x80,
	[0xC0 ... 0xFF]=0xFF
};

/* Header/BFPT fields: 32 MB, uniform 4 KB erase, 256-byte pages. */
static const uint8_t sfdp_32mb[0xAC] = {
	0x53,0x46,0x44,0x50,0x05,0x01,0x00,0xFF,0x00,0x05,0x01,0x10,0x80,0x00,0x00,0xFF,
	[0x80]=0xE5,0x20,0xF9,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	[0x9C]=0x0C,0x20,0x0F,0x52, /* erase type 1: 4 KB 0x20, type 2: 32 KB 0x52 */
	[0xA8]=0x82 /* page size: 2^8 */
};

/* Same image with a 128-byte page size in BFPT DWORD11. */
static const uint8_t sfdp_page128[0xAC] = {
	0x53,0x46,0x44,0x50,0x05,0x01,0x00,0xFF,0x00,0x05,0x01,0x10,0x80,0x00,0x00,0xFF,
	[0x80]=0xE5,0x20,0xF9,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	[0x9C]=0x0C,0x20,0x0F,0x52,
	[0xA8]=0x72 /* page size: 2^7 */
};

/* Same BFPT plus 4BAIT: fast read, page program, and 4 KB erase. */
static const uint8_t sfdp_32mb_4bait[0x108] = {
	0x53,0x46,0x44,0x50,0x05,0x01,0x01,0xFF,0x00,0x05,0x01,0x10,0x80,0x00,0x00,0xFF,
	0x84,0x00,0x01,0x02,0x00,0x01,0x00,0xFF,
	[0x80]=0xE5,0x20,0xF9,0xFF,0xFF,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	[0x9C]=0x0C,0x20,0x0F,0x52,
	[0xA8]=0x82,
	[0x100]=0x42,0x02,0x00,0x00,0x21 /* 4BAIT: bits 1, 6, 9; 4 KB opcode 0x21 */
};

/* BFPT has no uniform 4 KB erase type. */
static const uint8_t sfdp_nonuniform[0xAC] = {
	0x53,0x46,0x44,0x50,0x05,0x01,0x00,0xFF,0x00,0x05,0x01,0x10,0x80,0x00,0x00,0xFF,
	[0x80]=0xE4,0x20,0xF9,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	[0x9C]=0x10,0xD8,0xFF,0xFF
};

static bool SendCommand(SpiBus_t *bus, uint8_t opcode, uint32_t address,
	uint8_t address_bytes, const uint8_t *data, uint8_t data_len)
{
	uint8_t command[10];

	command[0] = opcode;
	for(uint8_t i = 0; i < address_bytes; i++)
		command[1 + i] = address >> (8 * (address_bytes - 1 - i));
	for(uint8_t i = 0; i < data_len; i++)
		command[1 + address_bytes + i] = data[i];
	return bus->SPI_WriteRead(bus->param, command,
		1 + address_bytes + data_len, NULL, 0);
}

static bool WaitNotBusy(SpiBus_t *bus)
{
	uint8_t status;

	for(uint8_t i = 0; i < FLASH_MOCK_MAX_BUSY_POLLS; i++)
	{
		uint8_t command = 0x05;
		if(!bus->SPI_WriteRead(bus->param, &command, 1, &status, 1))
			return false;
		if(!(status & 1))
			return true;
	}
	return false;
}

static bool CheckPageWrap(SpiFlashMock *mock, SpiBus_t *bus, uint32_t address)
{
	uint32_t wrap_offset = mock->Profile.PageSize - 2U;
	uint32_t erase_address = address - wrap_offset;
	uint8_t address_bytes = mock->Profile.Supports4ByteOpcodes ? 4 : 3;
	uint8_t program_opcode = mock->Profile.Supports4ByteOpcodes ? 0x12 : 0x02;
	uint8_t erase_opcode = mock->Profile.Supports4ByteOpcodes ? 0x21 : 0x20;
	static const uint8_t data[] = {0xF0, 0x0F, 0xAA, 0x55};

	if(!SendCommand(bus, 0x06, 0, 0, NULL, 0) ||
		!SendCommand(bus, erase_opcode, erase_address, address_bytes, NULL, 0) ||
		!WaitNotBusy(bus))
		return false;
	if(!SendCommand(bus, 0x06, 0, 0, NULL, 0) ||
		!SendCommand(bus, program_opcode, address, address_bytes, data, sizeof(data)) ||
		!WaitNotBusy(bus))
		return false;
	bool ok = mock->Memory[address] == 0xF0 && mock->Memory[address + 1] == 0x0F &&
		mock->Memory[erase_address] == 0xAA && mock->Memory[erase_address + 1] == 0x55;

	return SendCommand(bus, 0x06, 0, 0, NULL, 0) &&
		SendCommand(bus, erase_opcode, erase_address, address_bytes, NULL, 0) &&
		WaitNotBusy(bus) && ok;
}

static int RunProfile(const char *name, const SpiFlashMockProfile *profile,
	uint16_t start_sector)
{
	SpiFlashMock mock = {0};
	SpiBus_t bus = {0};
	NorDB_HWLayer *flash = NULL;
	NorDB_t *db = NULL;
	int result = EXIT_FAILURE;
	uint32_t start;
	uint32_t end;

	if(!SpiFlashMock_Init(&mock, profile))
		return EXIT_FAILURE;
	bus.param = &mock;
	bus.SPI_WriteRead = SpiFlashMock_WriteRead;
	flash = FlashDB_Init(start_sector, 4, &bus);
	if(!flash)
		goto cleanup;
	db = NorDB(flash, GetDummyRecordSize());
	if(!db)
		goto cleanup;
	start = (uint32_t)start_sector * flash->SectorSize;
	end = start + 4U * flash->SectorSize;
	RunFlashTests(db, 1);
	result = TestSummary();
	if(!CheckPageWrap(&mock, &bus, start + mock.Profile.PageSize - 2U))
	{
		printf("%s: page wrap check failed\n", name);
		result = EXIT_FAILURE;
	}
	for(uint32_t i = 0; i < mock.Profile.Size; i++)
	{
		if((i < start || i >= end) && mock.Memory[i] != 0xFF)
		{
			printf("%s: memory outside partition changed at 0x%08x\n", name, i);
			result = EXIT_FAILURE;
			break;
		}
	}
	if(mock.Violations != 0)
	{
		printf("%s: %u mock violations\n", name, mock.Violations);
		result = EXIT_FAILURE;
	}

cleanup:
	NorDB_Destroy(db);
	Flashll_Del(flash);
	printf("%s: %s, commands=%u reads=%u programs=%u erases=%u violations=%u\n",
		name, result == EXIT_SUCCESS ? "PASS" : "FAIL", mock.Commands,
		mock.Reads, mock.Programs, mock.Erases, mock.Violations);
	SpiFlashMock_Deinit(&mock);
	return result;
}

int main(void)
{
	static const SpiFlashMockProfile w25q64 = {
		{0xEF, 0x40, 0x17}, 8U * 1024U * 1024U, 0, false, 256,
		w25q64_sfdp, sizeof(w25q64_sfdp)
	};
	static const SpiFlashMockProfile w25q256 = {
		{0xEF, 0x40, 0x19}, 32U * 1024U * 1024U, 0, true, 256,
		sfdp_32mb_4bait, sizeof(sfdp_32mb_4bait)
	};
	static const SpiFlashMockProfile no_4byte = {
		{0xEF, 0x40, 0x19}, 32U * 1024U * 1024U, 0, false, 256,
		sfdp_32mb, sizeof(sfdp_32mb)
	};
	static const SpiFlashMockProfile page128 = {
		{0xEF, 0x40, 0x19}, 32U * 1024U * 1024U, 0, false, 128,
		sfdp_page128, sizeof(sfdp_page128)
	};
	static const SpiFlashMockProfile nonuniform = {
		{0xEF, 0x40, 0x17}, 8U * 1024U * 1024U, 0, false, 256,
		sfdp_nonuniform, sizeof(sfdp_nonuniform)
	};
	static const SpiFlashMockProfile unknown = {
		{0xEF, 0x40, 0x99}, 8U * 1024U * 1024U, 0, false, 256, NULL, 0
	};
	static const SpiFlashMockProfile known_fallback = {
		{0xEF, 0x40, 0x17}, 8U * 1024U * 1024U, 0, false, 256, NULL, 0
	};
	static const SpiFlashMockProfile gigadevice = {
		{0xC8, 0x40, 0x17}, 8U * 1024U * 1024U, 0, false, 256,
		w25q64_sfdp, sizeof(w25q64_sfdp)
	};
	SpiFlashMock mock = {0};
	SpiBus_t bus = {0};
	NorDB_HWLayer *flash;

	if(!SpiFlashMock_Init(&mock, &w25q64))
		return EXIT_FAILURE;
	bus.param = &mock;
	bus.SPI_WriteRead = SpiFlashMock_WriteRead;
	flash = FlashDB_Init(0, 1, &bus);
	if(!flash || bus.DevOnBus->ID != SfdpFlash || bus.DevOnBus->SectorCount != 2048 ||
		bus.DevOnBus->PageSize != 256 || bus.DevOnBus->SectorEraseOpcode != 0x20)
		return EXIT_FAILURE;
	Flashll_Del(flash);
	SpiFlashMock_Deinit(&mock);

	if(RunProfile("W25Q64", &w25q64, 0) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if(RunProfile("W25Q256", &w25q256, 4096 + 1) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if(RunProfile("32MB-clamped", &no_4byte, 0) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if(SpiFlashMock_Init(&mock, &page128))
	{
		bus = (SpiBus_t){.param = &mock, .SPI_WriteRead = SpiFlashMock_WriteRead};
		flash = FlashDB_Init(0, 1, &bus);
		if(!flash || bus.DevOnBus->PageSize != 128)
			return EXIT_FAILURE;
		Flashll_Del(flash);
		SpiFlashMock_Deinit(&mock);
	}
	else
		return EXIT_FAILURE;
	if(RunProfile("SFDP page128", &page128, 0) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if(SpiFlashMock_Init(&mock, &w25q64))
	{
		SpiFlashMock_FailSfdpTransfer(&mock, 1);
		bus = (SpiBus_t){.param = &mock, .SPI_WriteRead = SpiFlashMock_WriteRead};
		flash = FlashDB_Init(0, 1, &bus);
		if(flash)
			return EXIT_FAILURE;
		SpiFlashMock_Deinit(&mock);
	}
	else
		return EXIT_FAILURE;
	if(SpiFlashMock_Init(&mock, &no_4byte))
	{
		bus = (SpiBus_t){.param = &mock, .SPI_WriteRead = SpiFlashMock_WriteRead};
		flash = FlashDB_Init(4096 + 1, 4, &bus);
		if(flash)
			return EXIT_FAILURE;
		SpiFlashMock_Deinit(&mock);
	}
	else
		return EXIT_FAILURE;
	if(SpiFlashMock_Init(&mock, &nonuniform))
	{
		bus = (SpiBus_t){.param = &mock, .SPI_WriteRead = SpiFlashMock_WriteRead};
		flash = FlashDB_Init(0, 1, &bus);
		if(flash)
			return EXIT_FAILURE;
		SpiFlashMock_Deinit(&mock);
	}
	else
		return EXIT_FAILURE;
	if(SpiFlashMock_Init(&mock, &unknown))
	{
		bus = (SpiBus_t){.param = &mock, .SPI_WriteRead = SpiFlashMock_WriteRead};
		flash = FlashDB_Init(0, 1, &bus);
		if(flash)
			return EXIT_FAILURE;
		SpiFlashMock_Deinit(&mock);
	}
	else
		return EXIT_FAILURE;
	if(RunProfile("known fallback", &known_fallback, 0) != EXIT_SUCCESS ||
		RunProfile("GigaDevice SFDP", &gigadevice, 0) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
