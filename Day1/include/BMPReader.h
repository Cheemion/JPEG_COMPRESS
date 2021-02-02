#pragma once
#include "common.h"
#include <string>
class BMPReader {
public:
	BMPReader() = default;
	~BMPReader() {
		if (data) {
			delete[] data;
		}
	}
	bool open(std::string& path);
public:
	uint height = 0;
	uint width = 0;
	uint paddingBytes = 0;
	RGB* data = nullptr;
};

//2个字节对齐
#pragma pack(2)
typedef struct {
	unsigned short	bfType = 0x424d;
	unsigned int	bfSize = 0;
	unsigned short	bfReserved1 = 0;
	unsigned short	bfReserved2 = 0;
	unsigned int	bfOffBits = 0;
} BitMapFileHeader;

typedef struct {
	unsigned int	biSize;
	int				biWidth;
	int				biHeight;
	unsigned short	biPlanes;
	unsigned short	biBitCount;
	unsigned int	biCompression;
	unsigned int	biSizeImage;
	int				biXPelsPerMeter;
	int				biYPelsPerMeter;
	unsigned int	biClrUsed;
	unsigned int	biClrImportant;
} BitMapInfoHeader;

typedef struct {
	unsigned int 	bcSize;
	unsigned short  bcWidth;
	unsigned short  bcHeight;
	unsigned short  bcPlanes = 1;
	unsigned short  bcBitCount = 24;
} BitMapCoreHeader;
#pragma pack()