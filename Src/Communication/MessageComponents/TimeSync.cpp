#include "TimeSync.h"

size_t BNTPComponent::compress(char* buff) {
    return getSize();
}

bool BNTPComponent::decompress(char* compressed) {
    return true;
}

size_t BNTPComponent::getSize() {
    return 0;
}