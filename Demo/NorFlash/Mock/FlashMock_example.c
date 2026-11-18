#include <stdio.h>
#include <stdlib.h>

#include <NorDB.h>
#include <ll/SerialNorFlash_ll.h>
#include "TestRoutines.h"
#include "SpiFlashMock.h"

#define FLASH_MOCK_MAX_BUSY_POLLS 16

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
	uint8_t address_bytes = mock->Profile.Supports4ByteOpcodes ? 4 : 3;
	uint8_t program_opcode = mock->Profile.Supports4ByteOpcodes ? 0x12 : 0x02;
	uint8_t erase_opcode = mock->Profile.Supports4ByteOpcodes ? 0x21 : 0x20;
	static const uint8_t data[] = {0xF0, 0x0F, 0xAA, 0x55};

	if(!SendCommand(bus, 0x06, 0, 0, NULL, 0) ||
		!SendCommand(bus, erase_opcode, address - 0xFE, address_bytes, NULL, 0) ||
		!WaitNotBusy(bus))
		return false;
	if(!SendCommand(bus, 0x06, 0, 0, NULL, 0) ||
		!SendCommand(bus, program_opcode, address, address_bytes, data, sizeof(data)) ||
		!WaitNotBusy(bus))
		return false;
	bool ok = mock->Memory[address] == 0xF0 && mock->Memory[address + 1] == 0x0F &&
		mock->Memory[address - 0xFE] == 0xAA && mock->Memory[address - 0xFD] == 0x55;

	return SendCommand(bus, 0x06, 0, 0, NULL, 0) &&
		SendCommand(bus, erase_opcode, address - 0xFE, address_bytes, NULL, 0) &&
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
	if(!CheckPageWrap(&mock, &bus, start + 0xFE))
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
		{0xEF, 0x40, 0x17}, 8U * 1024U * 1024U, 0, false, 256, NULL, 0
	};
	static const SpiFlashMockProfile w25q256 = {
		{0xEF, 0x40, 0x19}, 32U * 1024U * 1024U, 0, true, 256, NULL, 0
	};

	if(RunProfile("W25Q64", &w25q64, 0) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	return RunProfile("W25Q256", &w25q256, 4096 + 1);
}
