#include "common.h"

void putInt(std::ostream& os, uint value) {
    os.put((value >> 0) & 0xFF); 
    os.put((value >> 8) & 0xFF); 
    os.put((value >> 16) & 0xFF); 
    os.put((value >> 24) & 0xFF); 
}
void putShort(std::ostream& os, unsigned short value) {
    os.put((value >> 0) & 0xFF); 
    os.put((value >> 8) & 0xFF); 
}