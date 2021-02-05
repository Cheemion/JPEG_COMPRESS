#include "JPG.h"
#include "common.h"

void JPG::convertToYCbCr() {
    for(uint i = 0; i < height; i++) {
        for(uint j = 0; j < width; j++) {
            YCbCr temp = BMPData[i * width + j];
            BMPData[i * width + j].Y  =  0.299  * temp.red + 0.587 * temp.green  + 0.114  * temp.blue;
            BMPData[i * width + j].Cb = -0.1687 * temp.red - 0.3313 * temp.green + 0.5    * temp.blue + 128;
            BMPData[i * width + j].Cr =  0.5    * temp.red - 0.4187 * temp.green - 0.0813 * temp.blue + 128;
        }
    }
}

void JPG::discreteCosineTransform() {

}
void JPG::quantization() {

}
void JPG::huffmanCoding() {

}
void JPG::output(std::string path) {

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