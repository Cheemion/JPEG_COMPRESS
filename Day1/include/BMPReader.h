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
	unsigned short	bfType;
	unsigned int	bfSize;
	unsigned short	bfReserved1;
	unsigned short	bfReserved2;
	unsigned int	bfOffBits;
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
	unsigned short  bcPlanes;
	unsigned short  bcBitCount;
} BitMapCoreHeader;

#pragma pack()