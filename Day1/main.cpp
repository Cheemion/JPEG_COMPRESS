#include <iostream>
#include "BMPReader.h"
#include <string>
#include <fstream>
//little-endian 输出
void putInt(std::ostream& os, uint value) {
    os.put((value >> 0) & 0xFF); 
    os.put((value >> 8) & 0xFF); 
    os.put((value >> 16) & 0xFF); 
    os.put((value >> 24) & 0xFF); 
}
void putShort(std::ostream& os, unsigned short value) {
    os.put((value >> 0) & 0xFF); 
    os.put((value >> 8) & 0xFF); 
}

std::ostream& outputBitMapFileHeader(std::ostream& os, const BitMapFileHeader& bitMapFileHeader) {
    os.put('B');
    os.put('M');
    putInt(os, bitMapFileHeader.bfSize);
    putShort(os, bitMapFileHeader.bfReserved1);
    putShort(os, bitMapFileHeader.bfReserved2);
    putInt(os, bitMapFileHeader.bfOffBits);
    return os;
}

std::ostream& outputBitMapCoreHeader(std::ostream& os, const BitMapCoreHeader& bitMapCoreHeader) {
    putInt(os, bitMapCoreHeader.bcSize);
    putShort(os, bitMapCoreHeader.bcWidth);
    putShort(os, bitMapCoreHeader.bcHeight);
    putShort(os, bitMapCoreHeader.bcPlanes);
    putShort(os, bitMapCoreHeader.bcBitCount);
    return os;
}

std::ostream& outputRGB(std::ostream& os, const RGB& rgb) {
    os.put(rgb.blue);
    os.put(rgb.green);
    os.put(rgb.red);
    return os;
}

int main() {

    std::string path(EXAMPLE_PATH_BMP);
    BMPReader bmpReader;
    if(!bmpReader.open(path)) {
        std::cout << "Error - canot open file\n" ;
        return 1;
    }

    std::string testPath = path.substr(0, path.find_last_of("\/")) + "\/test.bmp";
    std::fstream out(testPath, std::fstream::binary | std::fstream::out);

    if(!out) {
        std::cout << "Error - cannot open output file";
        return 1;
    }

    BitMapFileHeader bitMapFileHeader;
    bitMapFileHeader.bfSize = sizeof(BitMapFileHeader) + sizeof(BitMapCoreHeader) + bmpReader.width * bmpReader.height * 3 + bmpReader.width * bmpReader.paddingBytes;
    bitMapFileHeader.bfOffBits = sizeof(BitMapFileHeader) + sizeof(BitMapCoreHeader);
    outputBitMapFileHeader(out, bitMapFileHeader);

    BitMapCoreHeader bitMapCoreHeader;
    bitMapCoreHeader.bcSize = sizeof(BitMapCoreHeader);
    bitMapCoreHeader.bcHeight = bmpReader.height;
    bitMapCoreHeader.bcWidth = bmpReader.width;
    outputBitMapCoreHeader(out, bitMapCoreHeader);

    for(uint i = bmpReader.height - 1; i < bmpReader.height; i--) {
        for(uint j = 0; j < bmpReader.width; j++) {
            outputRGB(out, bmpReader.data[i * bmpReader.width + j]);
        }
        for(uint k = 0; k < bmpReader.paddingBytes; k++) {
            out.put(0);
        }
    }
    out.close();  
      
    return 0;
}