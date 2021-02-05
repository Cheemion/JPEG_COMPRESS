#pragma once
#include "BMPReader.h"
#include "common.h"
#include <algorithm>
using Block = double[64];

struct MCU {
    Block* y;
    Block* cb;
    Block* cr;
    //y的id为1, cb为2, cr*为3;
    Block*& operator[](uint id) {
        if(id == 1) {
            return y;
        } else if(id == 2) {
          return cb;  
        } else if(id == 3) {
            return cr;
        } else 
            throw std::runtime_error("Error - Block ID not existed:" + id);
    }
};

struct YCbCr {
    union
    {
        double Y;
        double red;
    };
    union 
    {
        double Cb;
        double green;
    };
    union {
        double Cr;
        double blue;
    };

    double operator[] (uint ID) {
        switch (ID) {
        case 1: return Y;
        case 2: return Cb;
        case 3: return Cr;
        default:
            throw std::runtime_error("Error - no such ID");
        }
    }
};

class JPG
{
public:
    void convertToYCbCr();
    void subsampling();
    void discreteCosineTransform();    
    void quantization();
    void huffmanCoding();
    void output(std::string path);
public:
    MCU* data;
    Block* blocks;
    YCbCr* BMPData;
    uint blockNum;

    //原图的像素
    uint width;
    uint height;

    //mcu 有多少个 长度是多少
    uint mcuWidth;
    uint mcuHeight;

    //一个完整的muc的水平和垂直像素个数
    uint mcuVerticalPixelNum;
    uint mcuHorizontalPixelNum;

    //用于subsampling
    // only support 1 or 2
    byte YVerticalSamplingFrequency;
    byte YHorizontalSamplingFrequency;
    byte CbVerticalSamplingFrequency;
    byte CbHorizontalSamplingFrequency;
    byte CrVerticalSamplingFrequency;
    byte CrHorizontalSamplingFrequency;
    byte maxVerticalSamplingFrequency;
    byte maxHorizontalSamplingFrequency;
public:
    uint getVerticalSamplingFrequency(uint ID) const{
        if(ID == 1) {
            return YVerticalSamplingFrequency;
        } else if(ID == 2) {
          return CbVerticalSamplingFrequency;  
        } else if(ID == 3) {
            return CrVerticalSamplingFrequency;
        } else 
            throw std::runtime_error("Error - Block ID not existed:" + ID);
    }
    uint getHorizontalSamplingFrequency(uint ID) const{
        if(ID == 1) {
            return YHorizontalSamplingFrequency;
        } else if(ID == 2) {
          return CbHorizontalSamplingFrequency;  
        } else if(ID == 3) {
            return CrHorizontalSamplingFrequency;
        } else 
            throw std::runtime_error("Error - Block ID not existed:" + ID);
    }
    
    JPG(uint width, uint height,const RGB* const rgbs,
        byte YVerticalSamplingFrequency, byte YHorizontalSamplingFrequency, 
        byte CbVerticalSamplingFrequency, byte CbHorizontalSamplingFrequency,
        byte CrVerticalSamplingFrequency, byte CrHorizontalSamplingFrequency
       )
        :width(width), height(height),
         YVerticalSamplingFrequency(YVerticalSamplingFrequency), YHorizontalSamplingFrequency(YHorizontalSamplingFrequency),
         CbVerticalSamplingFrequency(CbVerticalSamplingFrequency), CbHorizontalSamplingFrequency(CbHorizontalSamplingFrequency),
         CrVerticalSamplingFrequency(CrVerticalSamplingFrequency), CrHorizontalSamplingFrequency(CrHorizontalSamplingFrequency)
        {   
            maxHorizontalSamplingFrequency = std::max({YHorizontalSamplingFrequency, CbHorizontalSamplingFrequency, CrHorizontalSamplingFrequency});
            maxVerticalSamplingFrequency = std::max({YVerticalSamplingFrequency, CbVerticalSamplingFrequency, CrVerticalSamplingFrequency});
            //mcu的个数
            mcuWidth = (width + (maxHorizontalSamplingFrequency * 8 - 1)) / (maxHorizontalSamplingFrequency * 8);       
            mcuHeight = (height + (maxVerticalSamplingFrequency * 8 - 1)) / (maxVerticalSamplingFrequency * 8);   
            
            mcuVerticalPixelNum = maxVerticalSamplingFrequency * 8;
            mcuHorizontalPixelNum = maxHorizontalSamplingFrequency * 8;
            //总共多少个MCU    
            data = new MCU[mcuWidth * mcuHeight];
            //一个MCU有多少个Block
            blockNum = (YVerticalSamplingFrequency * YHorizontalSamplingFrequency + CbVerticalSamplingFrequency * CbHorizontalSamplingFrequency + CrHorizontalSamplingFrequency * CrVerticalSamplingFrequency);
           
            //分配内存空间
            blocks = new Block[mcuHeight * mcuHeight * blockNum];

            //把对于的内存
            for (uint i = 0; i < mcuHeight; i++) {
                for (uint j = 0; j < mcuWidth; j++) {
                    data[i * mcuWidth + j].y = &blocks[(i * mcuWidth + j) * blockNum];
                    data[i * mcuWidth + j].cb = data[i * mcuWidth + j].y + YVerticalSamplingFrequency * YHorizontalSamplingFrequency;
                    data[i * mcuWidth + j].cr = data[i * mcuWidth + j].cb + CbVerticalSamplingFrequency * CbHorizontalSamplingFrequency;
                }
            }

            BMPData = new YCbCr[width * height];
            for(uint i = 0; i < height; i++) {
                for(uint j = 0; j < width; j++) {
                    BMPData[i * width + j].red = static_cast<double>(rgbs[i * width + j].red);
                    BMPData[i * width + j].blue = static_cast<double>(rgbs[i * width + j].blue);
                    BMPData[i * width + j].green = static_cast<double>(rgbs[i * width + j].green);
                }
            }  
        }
    ~JPG() {
        delete[] data;
        delete[] blocks;
        delete[] BMPData;
    }

};
