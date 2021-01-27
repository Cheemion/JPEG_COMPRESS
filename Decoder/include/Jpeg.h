#pragma once
#include <vector>

typedef unsigned char byte;
typedef unsigned int uint;

// start of frame markers.
const byte SOF0 = 0xC0; //baseline
const byte SOF1 = 0xC1; //extended sequential
const byte SOF2 = 0xC2; //progressive
const byte SOF3 = 0xC3; //lossless

// start of frame marker differential
const byte SOF5 = 0xC5; //  sequential
const byte SOF6 = 0xC6; //  progressive
const byte SOF7 = 0xC7; //  lossless

// start of frame non differential, arithmetic coding
const byte SOF9 = 0xC9; // sequential
const byte SOF10 = 0xCA; // progressive
const byte SOF11 = 0xCB; // lossless

// start of frame differential, arithmetic coding
const byte SOF13 = 0xCD; //sequential
const byte SOF14 = 0xCE; // progressive
const byte SOF15 = 0xCF; // lossless

//define huffman table
const byte DHT = 0xC4;

//reserved
const byte JPG = 0xC8;

//define arithmetic coding conditions
const byte DAC = 0xCC;

//restart interval markers
const byte RST0 = 0xD0;
const byte RST1 = 0xD1;
const byte RST2 = 0xD2;
const byte RST3 = 0xD3;
const byte RST4 = 0xD4;
const byte RST5 = 0xD5;
const byte RST6 = 0xD6;
const byte RST7 = 0xD7;

//other markers
const byte SOI = 0xD8; //start of image
const byte EOI = 0xD9; //end of image
const byte SOS = 0xDA; // start of scan
const byte DQT = 0xDB; // define quantization talbe
const byte DNL = 0xDC; // number of line
const byte DRI = 0xDD; // restart interval
const byte DHP = 0xDE; // hierachical progression
const byte EXP = 0xDF; //expand reference component

//addition information, can by skipped
const byte APP0 = 0xE0;
const byte APP1 = 0xE1;
const byte APP2 = 0xE2;
const byte APP3 = 0xE3;
const byte APP4 = 0xE4;
const byte APP5 = 0xE5;
const byte APP6 = 0xE6;
const byte APP7 = 0xE7;
const byte APP8 = 0xE8;
const byte APP9 = 0xE9;
const byte APP10 = 0xEA;
const byte APP11 = 0xEB;
const byte APP12 = 0xEC;
const byte APP13 = 0xED;
const byte APP14 = 0xEE;
const byte APP15 = 0xEF;

//Misc Markers
const byte JPG0 = 0xF0;
const byte JPG1 = 0xF1;
const byte JPG2 = 0xF2;
const byte JPG3 = 0xF3;
const byte JPG4 = 0xF4;
const byte JPG5 = 0xF5;
const byte JPG6 = 0xF6;
const byte JPG7 = 0xF7;
const byte JPG8 = 0xF8;
const byte JPG9 = 0xF9;
const byte JPG10 = 0xFA;
const byte JPG11 = 0xFB;
const byte JPG12 = 0xFC;
const byte JPG13 = 0xFD;
const byte COM = 0xFE;
const byte TEM = 0x01;

struct HuffmanTable
{
	byte offsets[17] = { 0 };
	byte symbols[162] = { 0 };
	bool set = false;
};


const byte ZIG_ZAG[64] =
{
	0,   1,  8, 16, 9,  2,  3, 10,
	17, 24, 32, 25, 18, 11, 4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};
struct QuantizationTable {
	uint table[64] = { 0 };
	bool set = false;
};

struct ColorComponent {
	byte horizontalSamplingFactor = 1;
	byte verticalSamplingFactor = 1;
	byte quantizationTableID = 0;
	byte huffmanDCTableID = 0;
	byte huffmanACTableID = 0;
	bool used = false;
};

struct MCU {
	union {
		int y[64] = { 0 };
		int r[64];
	};
	union {
		int cb[64] = { 0 };
		int g[64];
	};	
	union {
		int cr[64] = { 0 };
		int b[64];
	};
};

struct Header
{	
	byte startOfSelection = 0;
	byte endOfSelection = 63;
	byte successiveApproximationHigh = 0;
	byte successiveApproximationLow = 0;
	std::vector<byte> huffmanData;
	HuffmanTable huffmanDCTables[4];
	HuffmanTable huffmanACTables[4];
	bool zeroBased = false;
	QuantizationTable quantizationTables[4];
	ColorComponent colorComponent[3];
	byte frameType = 0;
	uint height = 0;
	uint width = 0;
	byte numComponents = 0;
	uint restartInterval = 0;
	bool valid = true;
};

