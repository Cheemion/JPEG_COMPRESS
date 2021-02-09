#pragma once
#include "BMPReader.h"
#include "common.h"
#include <algorithm>
#include <vector>
// start of frame markers.
const byte SOF0 = 0xC0; //baseline
const byte SOF1 = 0xC1; //extended sequential
const byte SOF2 = 0xC2; //progressive
const byte SOF3 = 0xC3; //lossless

// start of frame marker differential
const byte SOF5 = 0xC5; //  sequential
const byte SOF6 = 0xC6; //  progressive
const byte SOF7 = 0xC7; //  lossless

// start of frame non differential, arithmetic coding
const byte SOF9 = 0xC9; // sequential
const byte SOF10 = 0xCA; // progressive
const byte SOF11 = 0xCB; // lossless

// start of frame differential, arithmetic coding
const byte SOF13 = 0xCD; //sequential
const byte SOF14 = 0xCE; // progressive
const byte SOF15 = 0xCF; // lossless

//define huffman table
const byte DHT = 0xC4;

//reserved
const byte JPG = 0xC8;

//define arithmetic coding conditions
const byte DAC = 0xCC;

//restart interval markers
const byte RST0 = 0xD0;
const byte RST1 = 0xD1;
const byte RST2 = 0xD2;
const byte RST3 = 0xD3;
const byte RST4 = 0xD4;
const byte RST5 = 0xD5;
const byte RST6 = 0xD6;
const byte RST7 = 0xD7;

//other markers
const byte SOI = 0xD8; //start of image
const byte EOI = 0xD9; //end of image
const byte SOS = 0xDA; // start of scan
const byte DQT = 0xDB; // define quantization talbe
const byte DNL = 0xDC; // number of line
const byte DRI = 0xDD; // restart interval
const byte DHP = 0xDE; // hierachical progression
const byte EXP = 0xDF; //expand reference component

//addition information, can by skipped
const byte APP0 = 0xE0;
const byte APP1 = 0xE1;
const byte APP2 = 0xE2;
const byte APP3 = 0xE3;
const byte APP4 = 0xE4;
const byte APP5 = 0xE5;
const byte APP6 = 0xE6;
const byte APP7 = 0xE7;
const byte APP8 = 0xE8;
const byte APP9 = 0xE9;
const byte APP10 = 0xEA;
const byte APP11 = 0xEB;
const byte APP12 = 0xEC;
const byte APP13 = 0xED;
const byte APP14 = 0xEE;
const byte APP15 = 0xEF;

//Misc Markers
const byte JPG0 = 0xF0;
const byte JPG1 = 0xF1;
const byte JPG2 = 0xF2;
const byte JPG3 = 0xF3;
const byte JPG4 = 0xF4;
const byte JPG5 = 0xF5;
const byte JPG6 = 0xF6;
const byte JPG7 = 0xF7;
const byte JPG8 = 0xF8;
const byte JPG9 = 0xF9;
const byte JPG10 = 0xFA;
const byte JPG11 = 0xFB;
const byte JPG12 = 0xFC;
const byte JPG13 = 0xFD;
const byte COM = 0xFE;
const byte TEM = 0x01;


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
struct Symbol {
    byte symbol;
    uint weight;
    uint codeLength;
    Symbol* nextSymbol;
    unsigned short code;
    Symbol() = default;
    Symbol(byte symbol, uint weight, uint codeLength, Symbol* nextSymbol) {
        this->symbol = symbol;
        this->weight = weight;
        this->codeLength = codeLength;
        this->nextSymbol = nextSymbol;
    }
};

struct LinkedSymbol {
    uint weight;
    Symbol* symbol;
};



class HuffmanTable {
public:
    std::vector<Symbol> sortedSymbol;

    //没有256个但是为了实现的方便,256个Symbol出现的次数
    uint countOfSymbol[256] = { 0 };
    //长度为i的code有多少个,去除0,这里保持了1到32位的code的个数
	byte codeCountOfLength[33] = { 0 };

    uint codeOfSymbol[256] = { 0 };
    uint codeLengthOfSymbol[256] = { 0 };
    //可以使用二叉树结构来生成哈夫曼
    //这边使用了书<Compressed Image File Formats JPEG, PNG, GIF, XBM, BMP - John Miano>66页中的实现方法
    // could use binary tree structure to generate hufman code for these symbols but for simpicility just use the method from book Compressed Image Files blablabla

    LinkedSymbol getLeastWeightLinkedSymbol(std::vector<LinkedSymbol>& symbols) {
            std::vector<LinkedSymbol>::size_type index = symbols.size() - 1;
            auto minWeight = symbols[index];
            //从底部开始循环
            for(auto i = symbols.size() - 1; i < symbols.size(); i--) {
                if(symbols[i].weight >= minWeight.weight) continue;
                minWeight = symbols[i];
                index = i;
            }
            symbols.erase(symbols.begin() + index);
            return minWeight;
    }

    static bool comp(const Symbol& a, const Symbol& b) {
        if(b.symbol == 0xFF) return true; //dummy symbol 永远在底部
        return a.codeLength < b.codeLength;
    }
    void generateHuffmanCode() {
        std::vector<LinkedSymbol> symbols;
        //遍历每个出现的symbol， add to vectors
        for(uint symbol = 0; symbol < 256; symbol++) {
            if(countOfSymbol[symbol] == 0) 
                continue;
            Symbol* s = new Symbol(symbol, countOfSymbol[symbol], 0, nullptr);
            LinkedSymbol linkedSymbol;
            linkedSymbol.symbol = s;
            linkedSymbol.weight = s->weight;
            symbols.push_back(linkedSymbol);
        }
        
        
        // FF是一个不会出现的symbol,作为我们的dummy symbol 防止one bit stream 的出现  比如11111, 这样就可以防止compressdata中出现FF的可能
        Symbol* dummySymbol = new Symbol(0xFF, 1, 0, nullptr); 
        LinkedSymbol dymmyLinkedSymbol;
        dymmyLinkedSymbol.symbol = dummySymbol;
        dymmyLinkedSymbol.weight = dummySymbol->weight;
        symbols.push_back(dymmyLinkedSymbol);
        
        

        //合并的过程
        while(symbols.size() != 1) {
            
            //leastWeight
            LinkedSymbol least = getLeastWeightLinkedSymbol(symbols);
            //second Least Weight
            LinkedSymbol second = getLeastWeightLinkedSymbol(symbols);
            //add two weights
            least.weight = least.weight + second.weight;

            //linked two linkedsymbols;
            Symbol* temp = second.symbol;
            while(temp->nextSymbol != nullptr)
                temp = temp->nextSymbol;
            temp->nextSymbol = least.symbol;
            least.symbol = second.symbol;
            //把每个symbol加1 codeLength,并且加入到 
            for(auto i = least.symbol; i != nullptr; i = i->nextSymbol) {
                i->codeLength++;
            }
            symbols.push_back(least);
        }

        //放入sortedSymbols
        for(Symbol* i = symbols[0].symbol; i != nullptr; i = i->nextSymbol) {
            sortedSymbol.push_back(*i);
        }

        //排序,并且把dummy symbol 放在最后面;
        std::sort(sortedSymbol.begin(), sortedSymbol.end(), comp);

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

        //规定codeLength不能大于16, 套用书上的方法实现了一下
        for(uint ii = 32; ii >= 17; ii--) {
            while(codeCountOfLength[ii] != 0) {
                uint jj = ii - 2;
                while(codeCountOfLength[jj] == 0)
                    jj--;
                codeCountOfLength[ii] = codeCountOfLength[ii] - 2;
                codeCountOfLength[ii - 1] = codeCountOfLength[ii - 1] + 1;
                codeCountOfLength[jj + 1] = codeCountOfLength[jj + 1] + 2;
                codeCountOfLength[jj] = codeCountOfLength[jj] - 1;
            }
        }

        uint index = 1; //codeLength赋值回去
        for (auto it = sortedSymbol.begin(); it != sortedSymbol.end(); it++) {
            if(codeCountOfLength[index] != 0) {
                it->codeLength = index;
                codeCountOfLength[index]--;
            } else {
                index++;
                it--;
            }
        }

        
        //生成huffmanCode for each symbol
        uint huffmanCode = 0;
        uint currentLength = 1;
        for (auto it = sortedSymbol.begin(); it != sortedSymbol.end(); it++) {
            if(currentLength == it->codeLength) {
                it->code = huffmanCode++;
                codeOfSymbol[it->symbol] = it->code;
                codeLengthOfSymbol[it->symbol] = it->codeLength;
            } else {
                huffmanCode = huffmanCode << 1;
                currentLength++;
                it--;
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
    HuffmanTable chromaAC;

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
    static const Block& getQuantizationTableByID(uint componentID);

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
