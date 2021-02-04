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
                //mcu有多少行个block
                uint blockRowNum = getHorizontalSamplingFrequency(componentID);
                //mcu有多少列个block
                uint blockColumnNum = getVerticalSamplingFrequency(componentID);
                //遍历block
                for(uint ii = 0; ii < blockRowNum; ii++, heightOffset+=8) {
                    for(uint jj = 0; jj < blockColumnNum; jj++, widthOffset+=8) {

                        Block& currentBlock = currentMCU[componentID][ii * blockColumnNum + jj];
                        //遍历Block every pixels 像素
                        for(uint y = 0; y < 8; y++) {
                            for(uint x = 0; x < 8; x++) {
                                uint sampledY = heightOffset + y *  maxVerticalSamplingFrequency / getVerticalSamplingFrequency(componentID);
                                uint sampledX = widthOffset + x * maxHorizontalSamplingFrequency / getHorizontalSamplingFrequency(componentID);
                                if(sampledY * width + sampledX >= width * height) break;
                                currentBlock[y * 8 + x] = BMPData[sampledY * width + sampledX][componentID];
                            }
                        }
                    }
                }             
            }
        }
    }  
} 