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
}


int main() {

    std::cout << "start" << std::endl;
    std::string path(RESOURCE_PATH);
    path = path + "example.bmp";
    BMPReader bmpReader; 
    if(!bmpReader.open(path)) {
        std::cout << "Error - canot open file\n" ;
        return 1;
    }
    
    JPG jpg(bmpReader.width, bmpReader.height, bmpReader.data, 2, 2, 1, 1, 1, 1) ;
    jpg.convertToYCbCr();
    jpg.subsampling();
    jpg.discreteCosineTransform();
    jpg.quantization();
    jpg.huffmanCoding();
    
    std::string jpgPath(path + "example.jpg");
    jpg.output(jpgPath);
    printJPG(jpg);
    std::cout << "end" << std::endl;
    return 0;
    
}