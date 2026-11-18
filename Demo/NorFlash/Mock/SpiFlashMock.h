#ifndef SPI_FLASH_MOCK_H
#define SPI_FLASH_MOCK_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	uint8_t JedecId[3];
	uint32_t Size;
	uint8_t Status1;
	bool Supports4ByteOpcodes;
	uint16_t PageSize;
	const uint8_t *Sfdp;
	uint32_t SfdpLength;
} SpiFlashMockProfile;

typedef struct
{
	SpiFlashMockProfile Profile;
	uint8_t *Memory;
	bool Wel;
	uint8_t Busy;
	uint32_t Commands;
	uint32_t Reads;
	uint32_t Programs;
	uint32_t Erases;
	uint32_t Violations;
	bool ViolationPrinted;
} SpiFlashMock;

bool SpiFlashMock_Init(SpiFlashMock *mock, const SpiFlashMockProfile *profile);
void SpiFlashMock_Deinit(SpiFlashMock *mock);
bool SpiFlashMock_WriteRead(void *param, uint8_t *out, uint16_t wlen,
	uint8_t *in, uint16_t rlen);

#endif
