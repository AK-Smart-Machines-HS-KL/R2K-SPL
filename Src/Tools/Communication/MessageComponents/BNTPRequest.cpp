#include "BNTPRequest.h"

size_t BNTPRequestComponent::compress(char* buff) {
    return getSize();
}

bool BNTPRequestComponent::decompress(char* compressed) {
    return true;
}

size_t BNTPRequestComponent::getSize() {
    return 0;
}