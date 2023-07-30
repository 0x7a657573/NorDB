
#include "stdafx.h"

#include <memory.h>

// #include "../include/libusb.h"

#include "ch341.h"

#include <libusb-1.0/libusb.h>

static struct libusb_device_handle *CH341DeviceHanlde;

unsigned char BitSwapTable(unsigned char c)
{
    static const unsigned char reverse_table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff};
    return reverse_table[c];
}


bool CH341DeviceInit(void)
{
	int ret;
	unsigned char desc[0x12];

	if (CH341DeviceHanlde)
		return true;

	if ((ret = libusb_init(NULL)))
	{
		fprintf(stderr, "Error: libusb_init failed: %d (%s)\n", ret, libusb_error_name(ret));
		return false;
	}

	if (!(CH341DeviceHanlde = libusb_open_device_with_vid_pid(NULL, CH341_USB_VID, CH341_USB_PID)))
	{
		fprintf(stderr, "Error: CH341 device (%04x/%04x) not found\n", CH341_USB_VID, CH341_USB_PID);
		return false;
	}

#if !defined(_MSC_VER) && !defined(MSYS) && !defined(CYGWIN) && !defined(WIN32) && !defined(MINGW) && !defined(MINGW32)
	if (libusb_kernel_driver_active(CH341DeviceHanlde, 0))
	{
		if ((ret = libusb_detach_kernel_driver(CH341DeviceHanlde, 0)))
		{
			fprintf(stderr, "Error: libusb_detach_kernel_driver failed: %d (%s)\n", ret, libusb_error_name(ret));
			goto cleanup;
		}
	}
#endif

	if ((ret = libusb_claim_interface(CH341DeviceHanlde, 0)))
	{
		fprintf(stderr, "Error: libusb_claim_interface failed: %d (%s)\n", ret, libusb_error_name(ret));
		goto cleanup;
	}

	if (!(ret = libusb_get_descriptor(CH341DeviceHanlde, LIBUSB_DT_DEVICE, 0x00, desc, 0x12)))
	{
		fprintf(stderr, "Warning: libusb_get_descriptor failed: %d (%s)\n", ret, libusb_error_name(ret));
	}

	printf("CH341 %d.%02d found.\n\n", desc[12], desc[13]);

	return true;

cleanup:
	libusb_close(CH341DeviceHanlde);
	CH341DeviceHanlde = NULL;
	return false;
}

void CH341DeviceRelease(void)
{
	if (!CH341DeviceHanlde)
		return;

	libusb_release_interface(CH341DeviceHanlde, 0);
	libusb_close(CH341DeviceHanlde);
	libusb_exit(NULL);

	CH341DeviceHanlde = NULL;
}

static int CH341USBTransferPart(enum libusb_endpoint_direction dir, unsigned char *buff, unsigned int size)
{
	int ret, bytestransferred;

	if (!CH341DeviceHanlde)
		return 0;

	if ((ret = libusb_bulk_transfer(CH341DeviceHanlde, CH341_USB_BULK_ENDPOINT | dir, buff, size, &bytestransferred, CH341_USB_TIMEOUT)))
	{
		fprintf(stderr, "Error: libusb_bulk_transfer for IN_EP failed: %d (%s)\n", ret, libusb_error_name(ret));
		return -1;
	}

	return bytestransferred;
}

static bool CH341USBTransfer(enum libusb_endpoint_direction dir, unsigned char *buff, unsigned int size)
{
	int pos, bytestransferred;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341USBTransferPart(dir, buff + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

#define CH341USBRead(buff, size) CH341USBTransfer(LIBUSB_ENDPOINT_IN, buff, size)
#define CH341USBWrite(buff, size) CH341USBTransfer(LIBUSB_ENDPOINT_OUT, buff, size)



bool CH341ChipSelect(unsigned int cs, bool enable)
{
	unsigned char pkt[4];

	static const int csio[4] = {0x36, 0x35, 0x33, 0x27};

	if (cs > 3)
	{
		fprintf(stderr, "Error: invalid CS pin %d, 0~3 are available\n", cs);
		return false;
	}

	pkt[0] = CH341_CMD_UIO_STREAM;
	if (enable)
		pkt[1] = CH341_CMD_UIO_STM_OUT | csio[cs];
	else
		pkt[1] = CH341_CMD_UIO_STM_OUT | 0x37;
	pkt[2] = CH341_CMD_UIO_STM_DIR | 0x3F;
	pkt[3] = CH341_CMD_UIO_STM_END;

	return CH341USBWrite(pkt, 4);
}

static int CH341TransferSPI(const unsigned char *in, unsigned char *out, unsigned int size)
{
	unsigned char pkt[CH341_PACKET_LENGTH];
	unsigned int i;

	if (!size)
		return 0;

	if (size > CH341_PACKET_LENGTH - 1)
		size = CH341_PACKET_LENGTH - 1;

	pkt[0] = CH341_CMD_SPI_STREAM;

	for (i = 0; i < size; i++)
		pkt[i + 1] = BitSwapTable(in[i]);

	if (!CH341USBWrite(pkt, size + 1))
	{
		fprintf(stderr, "Error: failed to transfer data to CH341\n");
		return -1;
	}

	if (!CH341USBRead(pkt, size))
	{
		fprintf(stderr, "Error: failed to transfer data from CH341\n");
		return -1;
	}

	for (i = 0; i < size; i++)
		out[i] = BitSwapTable(pkt[i]);

	return size;
}

bool CH341StreamSPI(const unsigned char *in, unsigned char *out, unsigned int size)
{
	int pos, bytestransferred;

	if (!size)
		return true;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(in + pos, out + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

bool CH341ReadSPI(unsigned char *out, unsigned int size)
{
	int pos, bytestransferred;
	unsigned char pkt[CH341_PACKET_LENGTH];

	if (!size)
		return true;

	memset(pkt, 0, sizeof (pkt));

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(pkt, out + pos, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}

bool CH341WriteSPI(const unsigned char *in, unsigned int size)
{
	int pos, bytestransferred;
	unsigned char pkt[CH341_PACKET_LENGTH+1] = {0};

	if (!size)
		return true;

	pos = 0;

	while (size)
	{
		bytestransferred = CH341TransferSPI(in + pos, pkt, size);

		if (bytestransferred <= 0)
			return false;

		pos += bytestransferred;
		size -= bytestransferred;
	}

	return true;
}
