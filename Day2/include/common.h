#pragma once
#include <iostream>
using byte = unsigned char;
using uint = unsigned int;
struct RGB {
	byte blue;
	byte green;
	byte red;
};

//little-endian 输出
void putInt(std::ostream& os, uint value);
void putShort(std::ostream& os, unsigned short value);
