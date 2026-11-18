#include "SpiFlashMock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPI_FLASH_MOCK_BUSY_POLLS 2

typedef enum
{
	COMMAND_STATUS,
	COMMAND_ID,
	COMMAND_WREN,
	COMMAND_READ_DUMMY,
	COMMAND_PROGRAM,
	COMMAND_ERASE,
	COMMAND_SFDP
} CommandClass;

static void MockViolation(SpiFlashMock *mock, const char *reason)
{
	mock->Violations++;
	if(!mock->ViolationPrinted)
	{
		printf("Flash mock violation: %s\n", reason);
		mock->ViolationPrinted = true;
	}
}

static bool CommandInfo(uint8_t opcode, CommandClass *class, uint16_t *address_bytes)
{
	switch(opcode)
	{
	case 0x05:
		*class = COMMAND_STATUS;
		*address_bytes = 0;
		return true;
	case 0x9F:
		*class = COMMAND_ID;
		*address_bytes = 0;
		return true;
	case 0x06:
		*class = COMMAND_WREN;
		*address_bytes = 0;
		return true;
	case 0x0B:
		*class = COMMAND_READ_DUMMY;
		*address_bytes = 3;
		return true;
	case 0x0C:
		*class = COMMAND_READ_DUMMY;
		*address_bytes = 4;
		return true;
	case 0x02:
		*class = COMMAND_PROGRAM;
		*address_bytes = 3;
		return true;
	case 0x12:
		*class = COMMAND_PROGRAM;
		*address_bytes = 4;
		return true;
	case 0x20:
		*class = COMMAND_ERASE;
		*address_bytes = 3;
		return true;
	case 0x21:
		*class = COMMAND_ERASE;
		*address_bytes = 4;
		return true;
	case 0x5A:
		*class = COMMAND_SFDP;
		*address_bytes = 3;
		return true;
	default:
		return false;
	}
}

static uint32_t ReadAddress(const uint8_t *out, uint16_t address_bytes)
{
	uint32_t address = 0;
	for(uint16_t i = 0; i < address_bytes; i++)
		address = (address << 8) | out[1 + i];
	return address;
}

bool SpiFlashMock_Init(SpiFlashMock *mock, const SpiFlashMockProfile *profile)
{
	if(!mock || !profile || profile->Size == 0)
		return false;
	memset(mock, 0, sizeof(*mock));
	mock->Profile = *profile;
	if(mock->Profile.PageSize == 0)
		mock->Profile.PageSize = 256;
	mock->Memory = malloc(profile->Size);
	if(!mock->Memory)
		return false;
	memset(mock->Memory, 0xFF, profile->Size);
	return true;
}

void SpiFlashMock_Deinit(SpiFlashMock *mock)
{
	if(!mock)
		return;
	free(mock->Memory);
	mock->Memory = NULL;
}

void SpiFlashMock_FailSfdpTransfer(SpiFlashMock *mock, uint32_t transfer)
{
	if(mock)
		mock->FailSfdpTransfer = transfer;
}

bool SpiFlashMock_WriteRead(void *param, uint8_t *out, uint16_t wlen,
	uint8_t *in, uint16_t rlen)
{
	SpiFlashMock *mock = param;
	uint8_t opcode;
	uint16_t address_bytes;
	CommandClass class;
	uint32_t address;

	if(!mock || !out || wlen == 0)
		return false;
	opcode = out[0];
	mock->Commands++;
	if(mock->Busy && opcode != 0x05)
	{
		MockViolation(mock, "command while busy");
		return false;
	}

	if(!CommandInfo(opcode, &class, &address_bytes))
	{
		MockViolation(mock, "unknown opcode");
		return false;
	}
	switch(class)
	{
	case COMMAND_STATUS:
		if(wlen != 1 || rlen != 1 || !in)
		{
			MockViolation(mock, "wrong status command length");
			return false;
		}
		*in = mock->Profile.Status1 | (mock->Wel ? 0x02 : 0) |
			(mock->Busy ? 0x01 : 0);
		if(mock->Busy)
			mock->Busy--;
		return true;
	case COMMAND_ID:
		if(wlen != 1 || rlen != 3 || !in)
		{
			MockViolation(mock, "wrong JEDEC command length");
			return false;
		}
		memcpy(in, mock->Profile.JedecId, 3);
		return true;
	case COMMAND_WREN:
		if(wlen != 1 || rlen != 0)
		{
			MockViolation(mock, "wrong write-enable command length");
			return false;
		}
		mock->Wel = true;
		return true;
	case COMMAND_READ_DUMMY:
	case COMMAND_PROGRAM:
	case COMMAND_ERASE:
	case COMMAND_SFDP:
		break;
	}
	if((opcode == 0x0C || opcode == 0x12 || opcode == 0x21) &&
		!mock->Profile.Supports4ByteOpcodes)
	{
		MockViolation(mock, "unsupported 4-byte opcode");
		return false;
	}
	switch(class)
	{
	case COMMAND_STATUS:
	case COMMAND_ID:
	case COMMAND_WREN:
		goto command_length_ok;
	case COMMAND_READ_DUMMY:
	case COMMAND_SFDP:
		if(wlen != 2 + address_bytes)
			break;
		goto command_length_ok;
	case COMMAND_ERASE:
		if(wlen != 1 + address_bytes)
			break;
		goto command_length_ok;
	case COMMAND_PROGRAM:
		if(wlen > 1 + address_bytes)
			goto command_length_ok;
		break;
	default:
		break;
	}
	{
		MockViolation(mock, "wrong command length");
		return false;
	}

command_length_ok:
	if(class == COMMAND_PROGRAM)
	{
		address = ReadAddress(out, address_bytes);
		if(address >= mock->Profile.Size)
		{
			MockViolation(mock, "program address beyond chip");
			return false;
		}
		if(!mock->Wel)
			MockViolation(mock, "program without write enable");
		if(mock->Wel && !(mock->Profile.Status1 & 0x1C))
		{
			uint32_t page = address - address % mock->Profile.PageSize;
			uint32_t offset = address % mock->Profile.PageSize;
			for(uint32_t i = 0; i < (uint32_t)(wlen - 1 - address_bytes); i++)
			{
				uint32_t target = page + (offset + i) % mock->Profile.PageSize;
				if(target >= mock->Profile.Size)
					MockViolation(mock, "program address beyond chip");
				else
					mock->Memory[target] &= out[1 + address_bytes + i];
			}
			mock->Programs++;
			mock->Busy = SPI_FLASH_MOCK_BUSY_POLLS;
		}
		mock->Wel = false;
		return true;
	}
	address = ReadAddress(out, address_bytes);
	if(class == COMMAND_ERASE)
	{
		if(address >= mock->Profile.Size || address + 0x1000U > mock->Profile.Size)
		{
			MockViolation(mock, "erase address beyond chip");
			return false;
		}
		if(!mock->Wel)
			MockViolation(mock, "erase without write enable");
		if(mock->Wel && !(mock->Profile.Status1 & 0x1C))
		{
			memset(mock->Memory + address, 0xFF, 0x1000);
			mock->Erases++;
			mock->Busy = SPI_FLASH_MOCK_BUSY_POLLS;
		}
		mock->Wel = false;
		return true;
	}
	if(class == COMMAND_SFDP)
	{
		mock->SfdpTransfers++;
		if(mock->SfdpTransfers == mock->FailSfdpTransfer)
			return false;
		if(!in && rlen)
			return false;
		for(uint16_t i = 0; i < rlen; i++)
			in[i] = mock->Profile.Sfdp && address + i < mock->Profile.SfdpLength ?
				mock->Profile.Sfdp[address + i] : 0xFF;
		return true;
	}
	if(class != COMMAND_READ_DUMMY || address >= mock->Profile.Size ||
		rlen > mock->Profile.Size - address)
	{
		MockViolation(mock, "read address beyond chip");
		return false;
	}
	if(!in && rlen)
		return false;
	memcpy(in, mock->Memory + address, rlen);
	mock->Reads++;
	return true;
}
