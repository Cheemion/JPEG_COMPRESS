#include <iostream>
#include "JPG.h" 
#include "BMPReader.h"

void printJPG(const JPG& jpg) {
    std::cout << "width:" << jpg.width << " " << "height:" << jpg.height << "\n";
    for (uint componentID = 1; componentID <= 3; componentID++) {
        std::cout << "ComponentID:" << componentID << "\n";
        for (uint i = 0; i < jpg.mcuHeight; i++) {
            for (uint j = 0; j < jpg.mcuWidth; j++) {
                MCU& mcu = jpg.data[i * jpg.mcuWidth + j];
                std::cout << "muc Row:" << i << " mcu column:" << j << "\n";
                for (uint ii = 0; ii < jpg.getVerticalSamplingFrequency(componentID); ii++) {
                    for (uint jj = 0; jj < jpg.getHorizontalSamplingFrequency(componentID); jj++) {
                        std::cout << "Block row:" << ii << " Block column:" << jj << "\n";
                        Block& block = mcu[componentID][ii * jpg.getHorizontalSamplingFrequency(componentID) + jj];
                        for(uint k = 0; k < 8; k++) {
                            for(uint q = 0; q < 8; q++) {
                                std::cout << block[k * 8 + q] << " ";
                            }
                            std::cout << "\n";
                        }
                    }
                }
            }
        }
    }
    std::cout << "DC Tables 0 "<< "\n";
    for(uint i = 1; i <= 16; i++) {
        std::cout << "length:" << i << ":::";
        for(auto it = jpg.yDC.sortedSymbol.cbegin(); it != jpg.yDC.sortedSymbol.cend(); it++) {
            if(it->codeLength == i) {
                std::cout << (uint)it->symbol << "  ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "DC Tables 1 "<< "\n";
    for(uint i = 1; i <= 16; i++) {
        std::cout << "length:" << i << ":::";
        for(auto it = jpg.chromaDC.sortedSymbol.cbegin(); it != jpg.chromaDC.sortedSymbol.cend(); it++) {
            if(it->codeLength == i) {
                std::cout << (uint)it->symbol << "  ";
            }
        }
        std::cout << "\n";

    }
    std::cout << "AC Tables 0 "<< "\n";
    for(uint i = 1; i <= 16; i++) {
        std::cout << "length:" << i << ":::";
        for(auto it = jpg.yAC.sortedSymbol.cbegin(); it != jpg.yAC.sortedSymbol.cend(); it++) {
            if(it->codeLength == i) {
                std::cout << (uint)it->symbol << "  ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "AC Tables 1 "<< "\n";
    for(uint i = 1; i <= 16; i++) {
        std::cout << "length:" << i << ":::";
        for(auto it = jpg.chromaAC.sortedSymbol.cbegin(); it != jpg.chromaAC.sortedSymbol.cend(); it++) {
            if(it->codeLength == i) {
                std::cout << (uint)it->symbol << "  ";
            }
        }
        std::cout << "\n";
    }
}



int main() {
    std::cout << "start" << std::endl;
    std::string path(RESOURCE_PATH);
    std::string inputFile = path + "lina.bmp";
    BMPReader bmpReader; 
    if(!bmpReader.open(inputFile)) {
        std::cout << "Error - canot open file\n" ;
        return 1;
    }

    JPG jpg(bmpReader.width, bmpReader.height, bmpReader.rgbData, 2, 2, 1, 1, 1, 1) ;
    jpg.convertToYCbCr();
    jpg.subsampling();
    
    jpg.discreteCosineTransform();
    jpg.quantization();
    jpg.huffmanCoding();

    std::string outputFile = path + "test.jpg";
    jpg.output(outputFile);

    printJPG(jpg);
    std::cout << "end" << std::endl;
    return 0;
}