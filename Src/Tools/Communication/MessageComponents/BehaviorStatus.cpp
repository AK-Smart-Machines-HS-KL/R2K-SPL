#include "BehaviorStatus.h"


size_t BehaviorStatusComponent::compress(uint8_t* buff) {
    size_t byteOffset = 0;
    
    memcpy(buff + byteOffset, &activity, sizeof(activity));
    byteOffset += sizeof(activity);
    return byteOffset;
}

bool BehaviorStatusComponent::decompress(uint8_t* compressed) {
    size_t byteOffset = 0;
    
    memcpy(&activity, compressed + byteOffset, sizeof(activity));
    byteOffset += sizeof(activity);
    return byteOffset == getSize();
}

size_t BehaviorStatusComponent::getSize() {
    return 1;
}