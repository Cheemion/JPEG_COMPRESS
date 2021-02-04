#include <iostream>
#include "JPG.h" 
#include "BMPReader.h"
int main() {
    std::cout << "hello" << std::endl;
    std::string path(EXAMPLE_PATH_BMP);
    BMPReader bmpReader;
    if(!bmpReader.open(path)) {
        std::cout << "Error - canot open file\n" ;
        return 1;
    }
    JPG jpg(bmpReader.width, bmpReader.height, bmpReader.data, 2, 2, 1, 1, 1, 1) ;
    jpg.convertToYCbCr();
   // jpg.subsampling();
   // jpg.discreteCosineTransform();
   // jpg.quantization();
   // jpg.huffmanCoding();
   // std::string jpgPath(EXAMPLE_PATH_JPG);
   // jpg.output(jpgPath);
    
    return 0;
}