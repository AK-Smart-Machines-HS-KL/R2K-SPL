#include "BNTPRequest.h"

size_t BNTPRequestComponent::compress(uint8_t* buff) {
    return getSize();
}

bool BNTPRequestComponent::decompress(uint8_t* compressed) {
    return true;
}

size_t BNTPRequestComponent::getSize() {
    return 0;
}