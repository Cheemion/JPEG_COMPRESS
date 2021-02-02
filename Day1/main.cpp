#include <iostream>
#include "BMPReader.h"
#include <string>
int main() {
    std::string path(EXAMPLE_PATH_BMP);
    BMPReader bmpReader;
    if(!bmpReader.open(path)) {
        std::cout << "Error - canot open file\n" ;
        return 1;
    }
    return 0;
}