#pragma once
#include <iostream>
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



//little-endian 输出
void putIntBigEndian(std::ostream& os, uint value);
void putShortBigEndian(std::ostream& os, unsigned short value);
