#include <iostream>
#include <fstream>
#include "Jpeg.h"
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
		std::cout << "Error opeing file" <<std::endl;
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


int main() {
	const std::string filename = "C:\\Users\\kim\\Desktop\\MMPlayer\\MPEG\\JPEG_COMPRESS\\lina.jpg";

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
	delete header;
	return 0;
}
