#include "JPG.h"
#include "common.h"
#include <cstring>
#include <cmath>

void JPG::convertToYCbCr() {
    for(uint i = 0; i < height; i++) {
        for(uint j = 0; j < width; j++) {
            YCbCr temp = BMPData[i * width + j];
            BMPData[i * width + j].Y  =  0.299  * temp.red + 0.587 * temp.green  + 0.114  * temp.blue;
            BMPData[i * width + j].Cb = -0.1687 * temp.red - 0.3313 * temp.green + 0.5    * temp.blue + 128;
            BMPData[i * width + j].Cr =  0.5    * temp.red - 0.4187 * temp.green - 0.0813 * temp.blue + 128;

            BMPData[i * width + j].Y = BMPData[i * width + j].Y > 255 ? 255 : BMPData[i * width + j].Y;
            BMPData[i * width + j].Y = BMPData[i * width + j].Y < 0 ? 0 : BMPData[i * width + j].Y;
            BMPData[i * width + j].Cb = BMPData[i * width + j].Cb > 255 ? 255 : BMPData[i * width + j].Cb;
            BMPData[i * width + j].Cb = BMPData[i * width + j].Cb < 0 ? 0 : BMPData[i * width + j].Cb;
            BMPData[i * width + j].Cr = BMPData[i * width + j].Cr > 255 ? 255 : BMPData[i * width + j].Cr;
            BMPData[i * width + j].Cr = BMPData[i * width + j].Cr < 0 ? 0 : BMPData[i * width + j].Cr;

        }
    }
}


//这里直接把左上的点 当作subsampling的点了
//也可以取平均值
void JPG::subsampling() {
    //遍历mcu
    for (uint i = 0; i < mcuHeight; i++) {
        for (uint j = 0; j < mcuWidth; j++) {
            MCU& currentMCU = data[i * mcuWidth + j];
            uint heightOffset = i * maxVerticalSamplingFrequency * 8;
            uint widthOffset = j * maxHorizontalSamplingFrequency * 8;
            //iterate over 每一个component Y, cb cr
            for (uint componentID = 1; componentID <= 3; componentID++) {
                //遍历block
                for(uint ii = 0, yOffSet = heightOffset; ii < getVerticalSamplingFrequency(componentID); ii++, yOffSet = yOffSet + 8) {
                    for(uint jj = 0, xOffset = widthOffset; jj < getHorizontalSamplingFrequency(componentID); jj++, xOffset = xOffset + 8) {
                        Block& currentBlock = currentMCU[componentID][ii * getHorizontalSamplingFrequency(componentID) + jj];
                        //遍历Block every pixels 像素
                        for(uint y = 0; y < 8; y++) {
                            for(uint x = 0; x < 8; x++) {
                                uint sampledY = yOffSet + y *  maxVerticalSamplingFrequency / getVerticalSamplingFrequency(componentID);
                                uint sampledX = xOffset + x * maxHorizontalSamplingFrequency / getHorizontalSamplingFrequency(componentID);
                                //cannot find in original pictures;
                                if(sampledX >= width || sampledY >= height) {
                                    currentBlock[y * 8 + x] = 0;
                                } else {
                                    currentBlock[y * 8 + x] = BMPData[sampledY * width + sampledX][componentID];
                                }
                            }
                        }
                    }
                }             
            }
        }
    }  
} 

void DCT(Block& block) {
    Block temp;
    std::memcpy(&temp, &block, sizeof(Block)); //copy from original
    //8 rows
    //DCT行变化
    for(uint i = 0; i < 8; i++) {
        int* f = &temp[i * 8]; //one dimension Array , and will perform DCT on it
        for(uint k = 0; k < 8; k++) {
            double sum = 0.0;
            for(uint n = 0; n < 8; n++){
                sum = sum + f[n] * std::cos(((n + 0.5) * M_PI * k / 8));
            }
            sum = (k == 0) ? (sum * std::sqrt(1.0 / 8)) : (sum * std::sqrt(2.0 / 8));
            block[i * 8 + k] = sum;
        }
    }
    
    std::memcpy(&temp, &block, sizeof(Block)); //copy from 
    //DCT列变化
    for(uint i = 0; i < 8; i++) {
        int* f = &temp[i]; //one dimension Array with 8 steps increment , and will perform DCT on it
        for(uint k = 0; k < 8; k++) {
            double sum = 0.0;
            for(uint n = 0; n < 8; n++){
                sum = sum + f[n * 8] * std::cos(((n + 0.5) * M_PI * k / 8));
            }
            sum = (k == 0) ? (sum * std::sqrt(1.0 / 8)) : (sum * std::sqrt(2.0 / 8));
            block[i + k * 8] = sum;
        }
    }
}

void JPG::discreteCosineTransform() {
    for (uint i = 0; i < mcuHeight; i++) {
        for (uint j = 0; j < mcuWidth; j++) {
            MCU& currentMCU = data[i * mcuWidth + j];
            //iterate over 每一个component Y, cb cr
            for (uint componentID = 1; componentID <= 3; componentID++) {
                //遍历block
                for(uint ii = 0; ii < getVerticalSamplingFrequency(componentID); ii++) {
                    for(uint jj = 0; jj < getHorizontalSamplingFrequency(componentID); jj++) {
                        Block& currentBlock = currentMCU[componentID][ii * getHorizontalSamplingFrequency(componentID) + jj];
                        //遍历Block every pixels 像素
                        //在进行DCT变化之前把0~255变化到-128~127
                        for(uint index = 0; index < 64; index++) {
                            currentBlock[index] = currentBlock[index] - 128;
                        }
                        DCT(currentBlock); 
                    }
                }             
            }
        }
    }  
}

void JPG::quantization() {
    for (uint i = 0; i < mcuHeight; i++) {
        for (uint j = 0; j < mcuWidth; j++) {
            MCU& currentMCU = data[i * mcuWidth + j];
            //iterate over 每一个component Y, cb cr
            for (uint componentID = 1; componentID <= 3; componentID++) {
                //遍历block
                for(uint ii = 0; ii < getVerticalSamplingFrequency(componentID); ii++) {
                    for(uint jj = 0; jj < getHorizontalSamplingFrequency(componentID); jj++) {
                        Block& currentBlock = currentMCU[componentID][ii * getHorizontalSamplingFrequency(componentID) + jj];
                        const Block& quantizationTable = getQuantizationTableByID(componentID);

                        for(uint index = 0; index < 64; index++) {
                             currentBlock[index] = currentBlock[index] / quantizationTable[index];
                        }
                    }
                }             
            }
        }
    }  
}

//从0->0 | (-1,1) -> 1 | (2,3,-3,-2) -> 2
byte getBinaryLengthByValue(int value) {
    value = value < 0 ? (-value) : value;
    byte result = 0;
    while(value != 0) {
        value = value >> 1;
        result = result + 1;
    }
    return result;    
}

void JPG::huffmanCoding() {
    //先创建Table
    //Y
    uint componentID = 1;
    int lastYDC = 0;
    yDC.identifer = 0;
    //创建YDC_Table
    for (uint i = 0; i < mcuHeight; i++) {
        for (uint j = 0; j < mcuWidth; j++) {
            MCU& currentMCU = data[i * mcuWidth + j];
            //iterate over 每一个component Y, cb cr
            //遍历block
            for(uint ii = 0; ii < getVerticalSamplingFrequency(componentID); ii++) {
                for(uint jj = 0; jj < getHorizontalSamplingFrequency(componentID); jj++) {
                    Block& currentBlock = currentMCU[componentID][ii * getHorizontalSamplingFrequency(componentID) + jj];
                    int difference = currentBlock[0] - lastYDC; //DC分量是encode difference
                    lastYDC = currentBlock[0];
                    byte symbol = getBinaryLengthByValue(difference); //Y的2进制的长度就是symbol的值
                    yDC.countOfSymbol[symbol]++;
                }
            }
            yDC.countOfSymbol[0xFF]++; // FF是一个不会出现的symbol,作为我们的dummy symbol 防止one bit stream 的出现  比如11111, 这样就可以防止compressdata中出现FF的可能,             
        }
    }
    
}


void JPG::output(std::string path) {

}
