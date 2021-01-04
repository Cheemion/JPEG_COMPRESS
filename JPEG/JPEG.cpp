#include "JpegEncoder.h"
#include <iostream>
int main()
{	
	const char* path = "C:\\Users\\kim\\Desktop\\MMPlayer\\MPEG\\JPEG_COMPRESS\\lina.bmp";
	JpegEncoder JpegEncoder;
	if (JpegEncoder.open(path) == -1) {
		printf("cannot open file");
		return -1;
	}

	return 0;
}
