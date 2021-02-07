#pragma once
#include "BMPReader.h"
#include "common.h"
#include <algorithm>


const byte ZIG_ZAG[64] =
{
	0,   1,  8, 16, 9,  2,  3, 10,
	17, 24, 32, 25, 18, 11, 4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

const Block QUANTIZATION_TABLE_Y = 
{
    16,  11,  10,  16,  24,   40,   51,   61,   
    12,  12,  14,  19,  26,   58,   60,   55,   
    14,  13,  16,  24,  40,   57,   69,   56,   
    14,  17,  22,  29,  51,   87,   80,   62,   
    18,  22,  37,  56,  68,   109,  103,  77,   
    24,  35,  55,  64,  81,   104,  113,  92,   
    49,  64,  78,  87,  103,  121,  120,  101,  
    72,  92,  95,  98,  112,  100,  103,  99
 };

const Block QUANTIZATION_TABLE_CBCR =
 {  
    17,  18,  24,  47,  99, 99,  99,  99,   
    18,  21,  26,  66,  99, 99,  99,  99,   
    24,  26,  56,  99,  99, 99,  99,  99,   
    47,  66,  99,  99,  99, 99,  99,  99,   
    99,  99,  99,  99,  99, 99,  99,  99,   
    99,  99,  99,  99,  99, 99,  99,  99,   
    99,  99,  99,  99,  99, 99,  99,  99,  
    99,  99,  99,  99,  99, 99,  99,  99  
};


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
        int Y;
        int red;
    };
    union 
    {
        int Cb;
        int green;
    };
    union {
        int Cr;
        int blue;
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

class HuffmanTable {
    struct Symbol {
        byte symbol;
        uint weight;
        uint codeLength;
        Symbol* nextSymbol = nullptr;
    };

    struct LinkedSymbol {
        uint weight;
        Symbol* symbol;
    };

	byte identifer;
    std::vector<Symbol> sortedSymbol;

    //没有256个但是为了实现的方便,256个Symbol出现的次数
    uint countOfSymbol[256] = { 0 };
    //长度为i的code要多少个
	byte codeCountOfLength[16] = { 0 };

    uint codeOfSymbol[256] = { 0 };

    //可以使用二叉树结构来生成哈夫曼
    //这边使用了书<Compressed Image File Formats JPEG, PNG, GIF, XBM, BMP - John Miano>66页中的实现方法
    // could use binary tree structure to generate hufman code for these symbols but for simpicility just use the method from book Compressed Image Files blablabla

    LinkedSymbol getLeastWeightLinkedSymbol(std::vector<LinkedSymbol>& symbols) {
            decltype(symbols.begin()) index = (symbols.end() - 1);
            auto minWeight = (symbols.end() - 1) -> weight;
            //从底部开始循环
            for(auto it = symbols.end(); it != symbols.begin(); it--) {
                if(it->weight > minWeight) continue;
                minWeight = it->weight;
                index = it;
            }
            auto leastWeighted = symbols.erase(index);
            return *leastWeighted;
    }

    void generateHuffmanCode() {
        std::vector<LinkedSymbol> symbols;
        //遍历每个出现的symbol， add to vectors
        for(byte symbol = 0; symbol < 256; symbol++) {
            if(countOfSymbol[symbol] == 0) 
                continue;
            Symbol* s = new Symbol{symbol, countOfSymbol[symbol], 0, nullptr};
            LinkedSymbol linkedSymbol;
            linkedSymbol.symbol = s;
            linkedSymbol.weight = s->weight;
            symbols.push_back(linkedSymbol);
        }

        //合并的过程
        while(symbols.size() != 1) {
            //leastWeight
            auto least = getLeastWeightLinkedSymbol(symbols);
            //second Least Weight
            auto secondLeast = getLeastWeightLinkedSymbol(symbols);
            //add two weights
            secondLeast.weight = secondLeast.weight + least.weight;
            //linked two linkedsymbols;
            Symbol* temp = secondLeast.symbol;
            while(temp->nextSymbol != nullptr)
                temp = temp->nextSymbol;
            temp->nextSymbol = least.symbol;
            //把每个symbol加1 codeLength,并且加入到 
            for(auto i = secondLeast.symbol; i != nullptr; i = i->nextSymbol) {
                i->codeLength++;
            }
        }
        //放入sortedSymbols
        for(Symbol* i = symbols[0].symbol; i != nullptr; i = i->nextSymbol) {
            sortedSymbol.push_back(*i);
        }
        
        //释放内存
        Symbol* temp = symbols[0].symbol;
        while(temp != nullptr) {
            auto t = temp->nextSymbol;
            delete temp;
            temp = t;
        }
        
        //长度为n的code的个数
        //生成codeLengthCount for each codeLength;
        for (auto it = sortedSymbol.cbegin(); it != sortedSymbol.cend(); it++) {
            codeCountOfLength[it->codeLength]++;
        }

        //规定codeLength不能大于16, 找到最大的codeLengtth
        while(sortedSymbol[sortedSymbol.size() - 1].codeLength > 16) {
            //找到小2的code,
            uint codeLengthTarget = sortedSymbol[sortedSymbol.size() - 1].codeLength - 2;
            for(auto i = sortedSymbol.size() - 1; i > 0; i--) {

            }
        }


        //生成huffmanCode for each symbol
        uint huffmanCode = 0;
        uint currentLength = 1;
        for (auto it = sortedSymbol.cbegin(); it != sortedSymbol.cend(); it++) {
            if(currentLength == it->codeLength) {
                codeOfSymbol[it->symbol] = huffmanCode++;
            } else {
                currentLength++;
                huffmanCode << 1;
            }
        }

    }
};

class JPG {
public:
    void convertToYCbCr();
    void subsampling();
    void discreteCosineTransform();    
    void quantization();
    void huffmanCoding();
    void output(std::string path);
    
public:
    HuffmanTable yDC;
    HuffmanTable yAC;
    HuffmanTable chromaDC;
    HuffmanTable chromaDC;

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
    //存放huffman的数据
    std::vector<byte> huffmanData;

public:

    static const Block& getQuantizationTableByID(uint componentID) {
        if(componentID == 1)
            return QUANTIZATION_TABLE_Y;
        else
            return QUANTIZATION_TABLE_CBCR;
    }
    
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
                    BMPData[i * width + j].red = static_cast<int>(rgbs[i * width + j].red);
                    BMPData[i * width + j].blue = static_cast<int>(rgbs[i * width + j].blue);
                    BMPData[i * width + j].green = static_cast<int>(rgbs[i * width + j].green);
                }
            }  
        }

    ~JPG() {
        delete[] data;
        delete[] blocks;
        delete[] BMPData;
    }

};
