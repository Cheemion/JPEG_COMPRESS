#pragma once



class JpegEncoder {
public:
	JpegEncoder();
	~JpegEncoder();
	int open(const char* path);
private:
	struct RGB
	{
		unsigned char blue;
		unsigned char green;
		unsigned char red;
	};

	int height;
	int width;
	int lineSize;
	RGB* data;
};