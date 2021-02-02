//#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> 
#include "BMPReader.h" 
 
bool BMPReader::open(std::string& path) {

	FILE* file = nullptr;
	if ((file = fopen(path.c_str(), "rb")) == nullptr) {
		printf("error occured when opening file:%s", path.c_str());
		return false;
	}

	BitMapFileHeader fileHeader;

	if (fread(&fileHeader, sizeof(fileHeader), 1, file) != 1) {
		printf("Error - error occured when reading BITMAPFILEHEADER");
		return false;
	}

	if(fileHeader.bfType != 0x4D42) {
		printf("Error - this is not a BMP file that you're reading");
		return false;
	}


	uint infoHeaderSize;
	fread(&infoHeaderSize, sizeof(infoHeaderSize), 1, file);
	fseek(file, -sizeof(infoHeaderSize), SEEK_CUR);
	//2中infoHeader, 通过size来判断
	if (infoHeaderSize == sizeof(BitMapCoreHeader)) {
		BitMapCoreHeader bitMapCoreHeader;
		if (fread(&bitMapCoreHeader, sizeof(bitMapCoreHeader), 1, file) != 1) {
			printf("Error - mal-structure BITMAPINFOHEADER");
			return false;
		}
		this->width = bitMapCoreHeader.bcWidth;
		this->height = bitMapCoreHeader.bcHeight;
		if (bitMapCoreHeader.bcBitCount != 24) {
			printf("Error - the picture format is not consistent with our programm");
			return false;
		}
	} else {
		BitMapInfoHeader bitMapInfoHeader;
		if (fread(&bitMapInfoHeader, sizeof(bitMapInfoHeader), 1, file) != 1) {
			printf("Error - mal-structure BITMAPINFOHEADER");
			return false;
		}
		this->width = bitMapInfoHeader.biWidth;
		this->height = bitMapInfoHeader.biHeight;
		if (bitMapInfoHeader.biBitCount != 24) {
			printf("Error - the picture format is not consistent with our programm");
			return false;
		}
	}
	//mutiple times of 4 bytes each row,
	// if width = 1, 1 * 3个像素 * 8位 = 24位, 差一个字节, paddingSize = 1
	// if width = 2, 2 * 3个 * 8位 = 48位  paddingSize = 2
	// if width = 3, paddingSize = 3
	// if width = 4, paddingSize = 0
	this->paddingBytes = width % 4;

	data = new (std::nothrow) RGB[this->width * this->height];
	if (!data) {
		printf("Error - error when allocating memroy for RGB");
		return false;
	}
	//跳到data数据的位置
	fseek(file, fileHeader.bfOffBits, SEEK_SET);
	for (uint i = 0; i < height; i++) {
		//read data
		if (width != fread(data + (height - 1 - i) * width , sizeof(RGB), width, file)) {
			printf("Error - something wrong when reading data from BMP file");
			delete data;
			return false;
		}
		fseek(file, paddingBytes, SEEK_CUR);
	}
	fclose(file);
	return true;
}