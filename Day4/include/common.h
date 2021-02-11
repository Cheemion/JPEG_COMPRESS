#pragma once
#include <iostream>
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif // !M_PI
using byte = unsigned char;
using uint = unsigned int;
struct RGB {
	byte blue;
	byte green;
	byte red;
};
using Block = int[64];

//little-endian 输出
void putInt(std::ostream& os, uint value);
void putShort(std::ostream& os, unsigned short value);
