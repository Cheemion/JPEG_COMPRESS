#include <iostream>
#include "Jpeg.h"
#include <fstream>


void readQuantizationTable(std::ifstream& inFile, Header* const header) {
	std::cout << "Reading DQT" << std::endl;
	int length = (inFile.get() << 8) + inFile.get();
	length = length - 2;
	while (length > 0) {	
		byte tableInfo = inFile.get();
		length = length - 1;
		byte tableID = tableInfo & 0x0F;

		//只能是 0 1 2 3 
		if (tableID > 3) {
			std::cout << "invaild quantizatio table ID:" << tableID << std::endl;
			header->valid = false;
			return;
		}

		header->quantizationTables[tableID].set = true;

		if (tableInfo >> 4 != 0) { //2bytes
			for (uint i = 0; i < 64; i++) {
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
	if (length - 1 != 0) {
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
		// usually 1 2 3
		// but 0 1 2 is possible
		// always force them into 1 2 3

		if (componentID == 0) {
			header->zeroBased = true;
		}

		if (header->zeroBased) {
			componentID += 1;
		}

		if (componentID == 0 || componentID > 3) {
			std::cout << "not supported compoenentID";
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
		component->horizontalSamplingFactor = samplingFactor & 0x0F;
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
			break;
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
			DQT的处理
			An DQT marker may appear anywhere within a JPEG file.
			one restriction is if a scan requires a quantization table
			it must have been defined in a previous DQT marker
		*/
		else if (current == DRI) {
			readRestartInterval(inFile, header);
		}
		
		last = inFile.get();
		current = inFile.get();
	}
	return header;
}


void printHeader(const Header* const header) {
	if (header == nullptr) return;
	std::cout << "DQT============= \n";
	for (uint i = 0; i < 4; i++) {
		if (header->quantizationTables[i].set) {
			std::cout << "Table ID:" << i << '\n';
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
	std::cout << "restrat interval:" << header->restartInterval << "\n";
}



int main() {
	const std::string filename = "C:\\Users\\kim\\Desktop\\MMPlayer\\MPEG\\JPEG_COMPRESS\\test.jpg";

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
	//decode Huffman data

	delete header;
	return 0;
}
