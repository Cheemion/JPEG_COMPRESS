#include <iostream>
#include <fstream>
#include "Jpeg.h"

class BitReader
{
private:
	uint nextByte = 0;
	uint nextBit = 0;
	const std::vector<byte>& data;
public:
	BitReader(const std::vector<byte>& d) : data(d) {}
	// read one bit (0 1) for return -1.
	int readBit() {
		if(nextByte >= data.size()) {
			return -1;
		}
		//一位一位的读取, 其实用0 1 2 4 或 是另外一种选择
		int bit = (data[nextByte] >> (7 - nextBit)) & 1;
		nextBit = nextBit + 1;
		//一个字节读完了
		if(nextBit == 8) {
			nextBit = 0;
			nextByte = nextByte + 1;
		}
		return bit;
	}

	int readBits(const uint length) {
		int bits = 0;
		for (uint i = 0; i < length; i++)
		{
			int bit = readBit();
			if (bit == -1) {
				bits = -1;
				break;
			}
			bits = (bits << 1) | bit;
		}
		return bits;
	}
	//if there are bits remaining,
	//advance to the 0th bit of the next byte
	void align() {
		if(nextByte >= data.size()) {
			return;
		}
		if (nextBit != 0) {
			nextBit = 0;
			nextByte = nextByte + 1;
		}
	}
};



//generate all huffman codes based on symbols from a huffman table
//长度加1 append 0
//assign后 再加1
void generateCodes(HuffmanTable& hTable) {
	uint code = 0;
	for(uint i = 0; i < 16; i++) {
		for (uint j = hTable.offsets[i]; j < hTable.offsets[i + 1]; j++) {
			hTable.codes[j] = code;
			code = code + 1;
		}
		code = code << 1;
	}
}

byte getNextSymbol(BitReader& b, const HuffmanTable& hTable) {
	uint currentCode = 0;
	for(uint i = 0; i < 16; i++) {
		int bit = b.readBit();
		if(bit == -1) {
			return -1; // -1 is 1111111111 in memory and 255 is not used in symbol
		}
		currentCode = (currentCode << 1) | bit;
		for (uint j = hTable.offsets[i]; j < hTable.offsets[i + 1]; j++) {
			if(currentCode == hTable.codes[j]) {
				return hTable.symbols[j];
			}
		}
	}
	return -1;
}

bool decodeMCUComponent(BitReader& b, int* const component,int& previousDC, const HuffmanTable& dcTable, const HuffmanTable& acTable) {
	//get the DC value for this MCU component
	// DC symbols 的 number of zeros 永远为0， length从0到11
	//DC symbol 几等于 DC length 因为0的个数为0
	byte length = getNextSymbol(b, dcTable);
	if(length == (byte)-1) {
		std::cout << "Error - Invalid DC value \n";
		return false;
	}
	// DC 从0 to 11
	if(length > 11) {
		std::cout << "Error - DC coefficient length greater than 11 \n";
		return false;
	}
	
	//length =0 return 0
	int coeff = b.readBits(length);
	if(coeff == -1) {
		std::cout << "Error - Invalid DC value \n";
		return false;
	}

	//长度1: 1, -1
	//length 2: 1 2 -2 -1
	//3:1 2 3 4, -4 -3 -2 -1
	if(length != 0 && coeff < (1 << (length - 1))) {
		coeff = coeff - ((1 << length) - 1);
	}
	component[0] = coeff + previousDC;
	previousDC = component[0];
	// get AC values
	uint i = 1;
	while(i < 64) {
		byte symbol = getNextSymbol(b, acTable);
		if(symbol == (byte)-1) {
			std::cout << "Erroor - Invalid AC value \n";
			return false;
		}
		//0x00 means fill reaminder of 0;
		if (symbol == 0x00) {
			for(; i < 64; i++) {
				component[ZIG_ZAG[i]] = 0;
			}
			return true;
		}
		//symbol 0xF0 means skip 16 0's
		byte numZeros = symbol == 0xF0 ? 16 : symbol >> 4;
		byte coeffLength = symbol & 0x0F;
		coeff = 0;
		if (i + numZeros >= 64) {
			std::cout << "Error - Zero run-length exceeded MCU\n";
			return false;
		}
		for (uint j = 0; j < numZeros; j++, i++)
		{
			component[ZIG_ZAG[i]] = 0;
		}
		if (coeffLength != 0) {
			coeff = b.readBits(coeffLength);
			if (coeff == -1)
			{
				std::cout << "Error - Invalid AC value\n";
				return false;
			}

			if(coeff < (1 << (length - 1))) {
				coeff = coeff - ((1 << length) - 1);
			}

			component[ZIG_ZAG[i]] = coeff;
			i++;
		}
	}
	return true;
}


MCU* decodeHuffmanData(Header* const header) {
	const uint mcuHeight = (header->height + 7) / 8;
	const uint mcuWidth = (header->width + 7) / 8;
	MCU* mcus = new (std::nothrow) MCU[mcuHeight * mcuWidth];
	if (mcus == nullptr)
	{
		std::cout << "Error - Memory error\n";
		return nullptr;
	}

	for (uint i = 0; i < 4; i++)
	{
		if(header->huffmanDCTables[i].set) {
			generateCodes(header->huffmanDCTables[i]);
		}
		if(header->huffmanACTables[i].set) {
			generateCodes(header->huffmanACTables[i]);
		}
	}

	BitReader b(header->huffmanData);
	int previousDCs[3] = { 0 };
	for (uint i = 0; i < mcuHeight * mcuWidth; i++) {
		for (uint j = 0; j < header->numComponents; j++) {	
			//restartInterval
			if(header->restartInterval != 0 && i % header->restartInterval == 0) {
				previousDCs[0] = previousDCs[1] = previousDCs[2] = 0;
				b.align();
			}
			//signal channel and signal mcu
			if(!decodeMCUComponent(b, mcus[i][j], previousDCs[j] ,header->huffmanDCTables[header->colorComponent[j].huffmanDCTableID], header->huffmanACTables[header->colorComponent[j].huffmanACTableID])) {
				delete[] mcus;
				return nullptr;
			}
		}
	}

	return mcus;	
}
void readQuantizationTable(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading DQT" << std::endl;
	int length = (inFile.get() << 8) + inFile.get();
	length = length - 2;
	while (length > 0) {	
		byte tableInfo = inFile.get();
		byte tableSize = tableInfo >> 4;
		byte tableID = tableInfo & 0x0F;
		length = length - 1;

		//只能是 0 1 2 3 
		if (tableID > 3) {
			std::cout << "invaild quantizatio table ID:" << tableID << std::endl;
			header->valid = false;
			return;
		}

		header->quantizationTables[tableID].set = true;

		if (tableSize != 0) { //2bytes
			for (uint i = 0; i < 64; i++) {
				//文件中的数据是以zig_zag的顺序存储的，所以需要以zig_zag的顺序赋值。
				header->quantizationTables[tableID].table[ZIG_ZAG[i]] = (inFile.get() << 8) + inFile.get();
			}
			length = length - 128;
		} else {
			for (uint i = 0; i < 64; i++) {
				header->quantizationTables[tableID].table[ZIG_ZAG[i]] = inFile.get();
			}
			length = length - 64;
		}
	}

	if (length != 0) {
		std::cout << "Error -DQT invalid\n";
		header->valid = false;
	}
}

void readHuffmanTable(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading DHT Markers\n";
	int length = (inFile.get() << 8) + inFile.get();
	length = length - 2;
	while (length > 0) {
		byte tableInfo = inFile.get();
		byte tableID = tableInfo & 0x0F;
		bool ACTable = tableInfo >> 4;
		if (tableID > 3) {
			std::cout << "Error Invalid Huffman talbe ID: " << tableID << std::endl;
			header->valid = false;
			return;
		}

		HuffmanTable* hTable;
		if (ACTable) {
			hTable = &(header->huffmanACTables[tableID]);
		} else {
			hTable = &(header->huffmanDCTables[tableID]);
		}
		hTable->set = true;
		hTable->offsets[0] = 0;
		uint allSymbols = 0;
		for (uint i = 1; i <= 16; i++) {
			allSymbols += inFile.get();
			hTable->offsets[i] = allSymbols;
		}
		/*
		AC不超过162种symbols
		number of zero 从 0-15, length of coeeficient 1 - 10
		plus F0 means 16个zero , 00 means all left that is zero
		
		DC 不超过12种
		number of zero 一直都是0
		length of coefficient 从0 到 11
		*/
			
		/*
			i dont think this is necessary, we can just throw a exception when encoutner an unknow symbols
			if (allSymbols > 162) {
				std::cout << "Error Invalid Huffman talbe ID: " << tableID << std::endl;
				header->valid = false;
				return;
			}
		*/

		for (uint i = 0; i < allSymbols; i++) {
			hTable->symbols[i] = inFile.get();
		}

		length = length - 17 - allSymbols;
	}

	if (length != 0) {
		std::cout << "Error - Invalid Huffman" << std::endl;
		header->valid = false;
	}
}

void readStartOfScan(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading SOS Marker\n";
	if (header->numComponents == 0) {
		std::cout << "Error - SOS detected before SOF\n";
		header->valid = false;
		return;
	}
	uint length = (inFile.get() << 8) + inFile.get();
	//后面判断其他东西的时候能用
	for (uint i = 0; i < header->numComponents; i++) {
		header->colorComponent[i].used = false;
	}

	byte numComponents = inFile.get();
	for (uint i = 0; i < numComponents; i++) {
		byte componentID = inFile.get();
		if (header->zeroBased) {
			componentID = componentID + 1;
		}
		if (componentID > header->numComponents) {
			std::cout << "Error - Invalid color component ID:" << (uint)componentID << std::endl;
			header->valid = false;
			return;
		}
		ColorComponent* component = &(header->colorComponent[componentID - 1]);
		if (component->used) {
			std::cout << "Error - Duplicate color component ID:" << (uint)componentID << std::endl;
			header->valid = false;
			return;
		}
		component->used = true;
		
		byte huffmanTableIDs = inFile.get();
		component->huffmanDCTableID = huffmanTableIDs >> 4;
		component->huffmanACTableID = huffmanTableIDs & 0x0F;
		if (component->huffmanDCTableID > 3 || component->huffmanACTableID > 3) {
			std::cout << "Error - Invalid huffmanTableID  ";
			std::cout << "DC Table ID:" << (uint)component->huffmanDCTableID << std::endl;
			std::cout << "AC Table ID:" << (uint)component->huffmanACTableID << std::endl;
			header->valid = false;
			return;
		}
	}

	header->startOfSelection = inFile.get();
	header->endOfSelection = inFile.get();
	byte successiveApproximation = inFile.get();

	header->successiveApproximationHigh = successiveApproximation >> 4;
	header->successiveApproximationLow = successiveApproximation & 0x0F;
	if (header->startOfSelection != 0 || header->endOfSelection != 63) {
		std::cout << "Error - Invalid spectral selection\n";
		header->valid = false;
		return;
	}
	if (header->successiveApproximationHigh != 0 || header->successiveApproximationLow != 0) {
		std::cout << "Error - Invalid successive approximation\n";
		header->valid = false;
		return;
	}

	if (length - 6 - (2 * numComponents) != 0) {
		std::cout << "Error - SOS invalid\n";
		header->valid = false;
	}
}

void readComment(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading COM MAKER\n";
	//big endian
	uint length = (inFile.get() << 8) + inFile.get();
	//读出不需要的数据
	for (uint i = 0; i < length - 2; i++) {
		inFile.get();
	}
}


void readAPPN(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading APPN MAKER\n";
	//big endian
	uint length = (inFile.get() << 8) + inFile.get();
	//读出不需要的数据
	for (uint i = 0; i < length - 2; i++) {
		inFile.get();
	}
}

void readRestartInterval(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading Restart Interval \n";
	uint length = (inFile.get() << 8) + inFile.get();
	header->restartInterval = (inFile.get() << 8) + inFile.get();
	if (length - 4 != 0) {
		std::cout << "Error read Restart Interval \n";
		header->valid = false;
	}
}

void readStartOfFrame(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading SOF Marker \n";
	//there can be only one SOF marker per JPEG file
	
	if (header->numComponents != 0) {
		std::cout << "Error mutiple SOFs\n";
		header->valid = false;
		return;
	}

	uint length = (inFile.get() << 8) + inFile.get();
	byte precision = inFile.get();

	//只支持8位的数据
	if (precision != 8) {
		std::cout << "invalid precision :" << precision << std::endl;
		header->valid = false;
		return;
	}

	header->height = (inFile.get() << 8) + inFile.get();
	header->width = (inFile.get() << 8) + inFile.get();

	if (header->height == 0 || header->width == 0) {
		std::cout << "zero width or height\n";
		header->valid = false;
		return;
	}
	
	header->numComponents = inFile.get();
	if (header->numComponents == 4) {
		std::cout << "not support CMYK color mode";
		header->valid = false;
		return;
	}

	if (header->numComponents == 0) {
		std::cout << "number of color components must not be zero";
		header->valid = false;
		return;
	}

	for (uint i = 0; i < header->numComponents; i++) {
		byte componentID = inFile.get();
		/*
			usually 1 2 3, but 0 1 2 is possible ,always force them into 1 2 3
			Y(1), 2(Cb), 3(Cr)
		*/
		if (componentID == 0) {
			header->zeroBased = true;
		}

		if (header->zeroBased) {
			componentID += 1;
		}

		if (componentID == 0 || componentID > 3) {
			std::cout << "not supported componentID:" << (uint)componentID << '\n';
			header->valid = false;
			return;
		}

		ColorComponent* component = &(header->colorComponent[componentID - 1]);
		if (component->used) {
			std::cout << "used color components";
			header->valid = false;
			return;
		}

		component->used = true;
		byte samplingFactor = inFile.get();
		component->horizontalSamplingFactor = samplingFactor >> 4;
		component->verticalSamplingFactor = samplingFactor & 0x0F;
		component->quantizationTableID = inFile.get();

		if (component->horizontalSamplingFactor != 1 || component->horizontalSamplingFactor != 1) {
			std::cout << "only support samplingFacotor that is 1";
			//header->valid = false;
			//return;
		}

		if (component->quantizationTableID > 3) {
			std::cout << "not valid quantization talbe id";
			header->valid = false;
			return;
		}
		//8是length2byte 加上 1byte sample precision 2bytes的 height 2 bytes的width  1byte的components                
		if (length - 8 - (3 * header->numComponents) != 0) {
			std::cout << "not consistent length";
			header->valid = false;
			return;
		}
	}
}

Header* readJPG(const std::string filename) {
	std::ifstream inFile = std::ifstream(filename, std::ios::in | std::ios::binary);
	if (!inFile) {
		std::cout << "Error opening file" <<std::endl;
		return nullptr;
	}

	Header* header = new(std::nothrow) Header;
	if (header == nullptr) {
		std::cout << "Memory error" << std::endl;
		inFile.close();
		return nullptr;
	}

	byte last = inFile.get();
	byte current = inFile.get();

	if (last != 0xff || current != SOI) {
		header->valid = false;
		inFile.close();
		std::cout << "not JPEG file" << std::endl;
		return header;
	}

	last = inFile.get();
	current = inFile.get();
	while (header->valid) {
		if (!inFile) {
			std::cout << "Error File ended premature" << std::endl;
			header->valid = false;
			inFile.close();
			return header;
		}

		if (last != 0xff) {
			std::cout << "Error expected a marker" << std::endl;
			header->valid = false;
			inFile.close();
			return header;
		}

		// 读取frame类型 只支持baseline
		if (current == SOF0) {
			header->frameType = SOF0;
			readStartOfFrame(inFile, header);
		}  
		/*
			APPn的处理
			An APPn marker may appear anywhere within a JPEG file.
			By convention, applications that create APPn markers store their name at the start of the marker to prevent conflicts with other applications.
		*/
		else if (current >= APP0 && current <= APP15) {
			readAPPN(inFile, header);
		}
		/*
			DQT的处理
			An DQT marker may appear anywhere within a JPEG file.
			one restriction is if a scan requires a quantization table
			it must have been defined in a previous DQT marker
		*/
		else if (current == DQT) {
			readQuantizationTable(inFile, header);
		} 
		/*
			DRI的处理
			Define Restart Interval marker specifies the number of MCUs 
			between restart markers within the compressed data.
			A DRI marker may appear anywhere in the file to define or redefine the restart interval.
			A DRI marker must appear somwhere in the file for a compressed data segment to include restart markers
			
		*/
		else if (current == DRI) {
			readRestartInterval(inFile, header);
		}
		/*
			DHT处理，哈夫曼表
		*/
		else if (current == DHT) {
			readHuffmanTable(inFile, header);
			
		}
		/*
			Start of Scan

		*/
		else if (current == SOS) {
			readStartOfScan(inFile, header);
			break;
		} //skip
		else if (current == COM){
			readComment(inFile, header);
		} //unnecessary, skip
		else if ((current >= JPG0 && current <= JPG13) || current == DNL || current == DHP || current == EXP) {
			readComment(inFile, header);
		}
		else if (current == TEM) {
			//TEM has no size
		}
		/*
			FF may be used as a fill character before the start of any marker
			just ignore
		*/
		else if (current == 0xFF) {
			current = inFile.get();
			continue;
		} 
		else if (current == SOI) {
			std::cout << "Error -Embedded JPGs not supported\n";
			header->valid = false;
			inFile.close();
			return header;
		} 
		else if (current == EOI) {
			std::cout << "Error - EOI detected before SOS\n";
			header->valid = false;
			inFile.close();
			return header;
		}
		else if (current == DAC) {
			std::cout << "Error - Arithmetic Coding code not supported\n";
			header->valid = false;
			inFile.close();
			return header;
		}
		else if (current >= SOF0 && current <= SOF15) {
			std::cout << "Error - SOF marker not supported: 0x" << std::hex << (uint)current << std::dec << std::endl;
			header->valid = false;
			inFile.close();
			return header;
		}
		else if (current >= RST0 && current <= RST7) {
			std::cout << "Error - RSTN detected before SOS \n";
			header->valid = false;
			inFile.close();
			return header;
		}
		else {
			std::cout << "Error - Unknow marker 0x:" << std::hex << (uint)current << std::dec << std::endl;
			header->valid = false;
			inFile.close();
			return header;
		}
		last = inFile.get();
		current = inFile.get();
	}

	/*
		after SOS
		压缩数据的处理
		FF00代表00
		FFFF 多个FF的话，只当做一个FF
		会遇到RST, DNL, EOI
	*/
	if (header->valid) {
		current = inFile.get();
		//read compressed image data
		while (true) {
			if (!inFile) {
				std::cout << "Error File ended premature" << std::endl;
				header->valid = false;
				inFile.close();
				return header;
			}
			last = current;
			current = inFile.get();
			//marker or FF
			if (last == 0xFF) {
				//end of image
				if (current == EOI) {
					break;
				} 
				// 0xFF00 means FF
				else if (current == 0x00) {
					header->huffmanData.push_back(last);
					current = inFile.get();
				}
				//restart marker
				else if (current >= RST0 && current <= RST7) {
					current = inFile.get();
				}
				//ignore multiple 0xFF
				else if (current == 0xFF) {
					// do nothing, one FF will be ignored.
					continue;
				}
				else {
					std::cout << "Error Invalid marker during compressed data scan: 0x" << std::hex << (uint)current << std::dec << std::endl;
					header->valid = false;
					inFile.close();
					return header;
				}
			}
			else {
				header->huffmanData.push_back(last);
			}
		}
	}

	//invalid header info
	if (header->numComponents != 1 && header->numComponents != 3) {
		std::cout << "Error - " << header->numComponents << "required color compoenent 1 or 3 \n";
		header->valid = false;
		inFile.close();
		return header;
	}

	for (uint i = 0; i < header->numComponents; i++) {
		if (header->quantizationTables[header->colorComponent[i].quantizationTableID].set == false) {
			std::cout << "Error - using uninitialized quantization talbe\n";
			header->valid = false;
			inFile.close();
			return header;
		}
		if (header->huffmanACTables[header->colorComponent[i].quantizationTableID].set == false) {
			std::cout << "Error - using uninitialized Huffman AC talbe\n";
			header->valid = false;
			inFile.close();
			return header;
		}
		if (header->huffmanDCTables[header->colorComponent[i].quantizationTableID].set == false) {
			std::cout << "Error - using uninitialized Huffman DC talbe\n";
			header->valid = false;
			inFile.close();
			return header;
		}
	}
	inFile.close();
	return header;
}

void printHeader(const Header* const header) {
	if (header == nullptr) return;
	std::cout << "DQT============= \n";
	for (uint i = 0; i < 4; i++) {
		if (header->quantizationTables[i].set) {
			std::cout << "Table ID: " << i << '\n';
			std::cout << "Table Data";
			for (uint j = 0; j < 64; j++) {
				if (j % 8 == 0) {
					std::cout << '\n';
				}
				std::cout << header->quantizationTables[i].table[j] << ' ';
			}
			std::cout << '\n';
		}
	}

	std::cout << "SOF###########################\n";
	std::cout << "Frame Type 0x" << std::hex << (uint)header->frameType << std::dec << '\n';
	std::cout << "Height:" << header->height <<"\n";
	std::cout << "width:" << header->width << "\n";
	for (uint i = 0; i < header->numComponents; i++) {
		std::cout << "Component ID:" << (i + 1) << '\n';
		std::cout << "Horizontal Sampling Factor" << (uint)header->colorComponent[i].horizontalSamplingFactor << '\n';
		std::cout << "vertical Sampling Factor" << (uint)header->colorComponent[i].verticalSamplingFactor << '\n';
		std::cout << "Quantization Table ID" << (uint)header->colorComponent[i].quantizationTableID << '\n';
	}
	std::cout << "DRI==================\n";
	std::cout << "restrat interval:" << header->restartInterval << "\n";
	
	std::cout << "DHT=========================\n";
	std::cout << "DC Tables\n";
	for (uint i = 0; i < 4; i++) {
		if (header->huffmanDCTables[i].set) {
			std::cout << "Table ID:" << i << "\n";
			std::cout << "Symbols:\n";
			for (uint j = 0; j < 16; j++) {
				std::cout << "length:"<< (j + 1) << ":";
				for (uint k = header->huffmanDCTables[i].offsets[j]; k < header->huffmanDCTables[i].offsets[j + 1]; k++) {
					std::cout << std::hex << (uint)header->huffmanDCTables[i].symbols[k] << " ";
				} 
			std::cout << "\n";
			}
		}
	}
	std::cout << "AC Tables\n";
	for (uint i = 0; i < 4; i++) {
		if (header->huffmanACTables[i].set) {
			std::cout << "Table ID:" << i << "\n";
			std::cout << "Symbols:\n";
			for (uint j = 0; j < 16; j++) {
				std::cout << "length:" << (j + 1) << ":";
				for (uint k = header->huffmanACTables[i].offsets[j]; k < header->huffmanACTables[i].offsets[j + 1]; k++) {
					std::cout << std::hex << (uint)header->huffmanACTables[i].symbols[k] << " ";
				}
			std::cout << "\n";
			}
		}
	}

	std::cout << "SOS======================\n";
	std::cout << "Start of Selection" << (uint)header->startOfSelection << "\n";
	std::cout << "end of Selection" << (uint)header->endOfSelection << "\n";
	std::cout << "Successive Approximation High" << (uint)header->successiveApproximationHigh << "\n";
	std::cout << "successive Approximation Low" << (uint)header->successiveApproximationLow << "\n";

	std::cout << "Color Compoenents\n";
	for (uint i = 0; i < header->numComponents; i++) {
		std::cout << "Compoenent ID: " << (i + 1) << '\n';
		std::cout << "Huffman DC Table ID:" << (uint)header->colorComponent[i].huffmanDCTableID << "\n";
		std::cout << "Huffman AC Table ID:" << (uint)header->colorComponent[i].huffmanACTableID << "\n";
		std::cout << "quantization Table ID:" << (uint)header->colorComponent[i].quantizationTableID << "\n";
	}

	std::cout << "Length of Huffman Data:" << header->huffmanData.size() << "\n";
}

//write a 4-byte integer in little-endian
void putInt(std::ofstream& outFile, const uint v) {
	outFile.put((v >> 0) & 0xFF);
	outFile.put((v >> 8) & 0xFF);
	outFile.put((v >> 16) & 0xFF);
	outFile.put((v >> 24) & 0xFF);
}


void putShort(std::ofstream& outFile, const uint v) {
	outFile.put((v >> 0) & 0xFF);
	outFile.put((v >> 8) & 0xFF);
}
void writeBMP(const Header* const header, const MCU* const mcus, const std::string outFileName) {
	//open file
	std::ofstream outFile = std::ofstream(outFileName, std::ios::out | std::ios::binary);
	if(!outFile.is_open()) {
		std::cout << "Error - Error opening output file\n";
		return;
	}

	const uint mcuHeight = (header->height + 7) / 8;
	const uint mcuWidth = (header->width + 7) / 8;

	//mutiple times of 4 bytes each row,
	// if width = 1, paddSize = 1
	// if width = 2, paddingsize = 2
	// if width = 3, paddingsize = 3
	// if width = 4, paddingSize = 0
	const uint paddingSize = header->width % 4;

	//File Header 14 bytes,
	//mapInfo Header 12 bytes;
	const uint size = 14 + 12 + header->height * header->width * 3 + paddingSize * header->height;

	//File Header
	//first two bytes
	outFile.put('B');
	outFile.put('M');
	//4bytes of the BMP file in bytes
	putInt(outFile, size);
	//reserved 4 bytes
	putInt(outFile, 0);
	//The offset of the byte where the bitmap image data can be found.
	putInt(outFile, 0x1A);

	//BITMAPINFOHeader contains info about the dimensions and colors
	putInt(outFile, 12);//num of bytes 
	putShort(outFile, header->width);// width 
	putShort(outFile, header->height);//height
	putShort(outFile, 1);// number of planes , must be 1
	putShort(outFile, 24);//number of bits-per-pixel

	//这里有一点就是 因为y是无符号数，所有y<header->height的判断条件可以不变
	for(uint y = header->height - 1; y < header->height; --y) { 
		const uint mcuRow = y / 8;
		const uint pixelRow = y % 8;
		for (uint x = 0; x < header->width; ++x)
		{
			const uint mcuColumn = x / 8;
			const uint pixelColumn = x % 8;
			const uint mcuIndex = mcuRow * mcuWidth + mcuColumn;
			const uint pixelIndex = pixelRow * 8 + pixelColumn;
			outFile.put(mcus[mcuIndex].b[pixelIndex]);
			outFile.put(mcus[mcuIndex].g[pixelIndex]);
			outFile.put(mcus[mcuIndex].r[pixelIndex]);
		}
		for (uint i = 0; i < paddingSize; i++)
		{
			outFile.put(0);
		}
	}
	outFile.close();
}

int main() {
	
	//jpeg file path
	const std::string filename(EXAMPLE_PATH);

	Header* header = readJPG(filename);
	if (header == nullptr) {
		return -1;
	}

	if (header->valid == false) {
		std::cout << "Error" << std::endl;
		delete header;
		return -1;
	}

	printHeader(header);
	std::cout << "done" << std::endl;

	//decode huffman data
	MCU* mcus = decodeHuffmanData(header);
	//write BMP file
	if (mcus == nullptr)
	{	
		delete header;
		return 0;
	}
	// write BMP file
	const std::size_t pos = filename.find_last_of('.');
	const std::string outFileName = (pos == std::string::npos) ? (filename + ".") : (filename.substr(0, pos) + ".bmp");
	writeBMP(header, mcus, outFileName);
	delete[] mcus;
	delete header;

	return 0;
}
