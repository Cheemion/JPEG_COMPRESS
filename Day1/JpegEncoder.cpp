#define _CRT_SECURE_NO_WARNINGS
#include "JpegEncoder.h"
#include <stdio.h> 
JpegEncoder::JpegEncoder():
	height(0), width(0), data(nullptr)
{
}
JpegEncoder::~JpegEncoder()
{
	if (data)
		delete data;
}

/*
	失败返回-1；
*/
int JpegEncoder::open(const char * path)
{	
//2字节对齐
#pragma pack(2)
	typedef struct {
		unsigned short	bfType;
		unsigned int	bfSize;
		unsigned short	bfReserved1;
		unsigned short	bfReserved2;
		unsigned int	bfOffBits;
	} BITMAPFILEHEADER;

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
	} BITMAPINFOHEADER;
#pragma pack()

	FILE* file = nullptr;
	if ((file = fopen(path, "rb")) == nullptr) {
		printf("error occured when opening file:%s", path);
		return -1;
	}
	BITMAPFILEHEADER fileHeader;

	if (fread(&fileHeader, sizeof(fileHeader), 1, file) != 1) {
		printf("error occured when reading BITMAPFILEHEADER");
		return -1;
	}
	if(fileHeader.bfType != 0x4D42) {
		printf("this is not a BMP file that you're reading");
		return -1;
	}

	BITMAPINFOHEADER infoHeader; 
	if (fread(&infoHeader, sizeof(infoHeader), 1, file) != 1) {
		printf("error occured when reading BITMAPINFOHEADER");
		return -1;
	}
	//不支持size=24的另外一个bmp格式
	//不支持biHeight小于0的处理方式
	//不支持压缩格式
	//只支持24k真彩色
	if (infoHeader.biBitCount != 24 || infoHeader.biCompression != 0 || infoHeader.biSize == 24 || infoHeader.biHeight < 0) {
		printf("the picture format is not consistent with our programm");
		return -1;
	}

	this->width = infoHeader.biWidth;
	this->height = infoHeader.biHeight;
	int bmpBytes = width * height * 3;
	//每行都得是4字节的整数倍
	this->lineSize = (width * 3 + 3) / 4 * 4;
	data = (RGB*)new unsigned char[bmpBytes];

	if (!data) {
		printf("something wrong when allocating memory");
		return -1;
	}
	/*
	some applications place filler bytes between
	these structures and the pixel data so you must use the bfOffbits to determine
	the number of bytes from the BITMAPFILEHEADER structure to the pixel data
	*/
	fseek(file, fileHeader.bfOffBits, SEEK_SET);

	for (int i = 0; i < height; i++) {
		if (width != fread((unsigned char*)data + (height - 1 - i) * lineSize, sizeof(RGB), width, file)) {
			printf("something wrong when reading data from BMP file");
			return -1;
		}
	}
	
	fclose(file);
	return 0;
}
