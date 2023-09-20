#include "BallModel.h"


size_t BallModelComponent::compress(uint8_t* buff) {
    return getSize();
}

bool BallModelComponent::decompress(uint8_t* compressed) {
    return true;
}

size_t BallModelComponent::getSize() {
    return 0;
}