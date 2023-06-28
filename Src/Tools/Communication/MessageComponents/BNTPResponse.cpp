#include "BNTPResponse.h"
#include <limits.h>
#include "Platform/BHAssert.h"

// n(1), [origination(4), reciept(4), reciever(1)] x n

size_t BNTPResponseComponent::compress(uint8_t* buff) {
    size_t byteOffset = 0;
    
    // size must be less than max(n)
    ASSERT(messages.size() < std::numeric_limits<uint8_t>::max());
    uint8_t n = (uint8_t) messages.size();

    // n first
    memcpy(buff + byteOffset, &n, sizeof(n));
    byteOffset += sizeof(n);

    for (auto &msg : messages)
    {
        memcpy(buff + byteOffset, &msg.requestOrigination, sizeof(msg.requestOrigination));
        byteOffset += sizeof(msg.requestOrigination);

        memcpy(buff + byteOffset, &msg.requestReceipt, sizeof(msg.requestReceipt));
        byteOffset += sizeof(msg.requestReceipt);

        memcpy(buff + byteOffset, &msg.receiver, sizeof(msg.receiver));
        byteOffset += sizeof(msg.receiver);
    }

    return byteOffset;
}

bool BNTPResponseComponent::decompress(uint8_t* compressed) {
    size_t byteOffset = 0;
    uint8_t n;

    messages.clear();

    memcpy(&n, compressed + byteOffset, sizeof(n));
    byteOffset += sizeof(n);

    for (size_t i = 0; i < n; i++)
    {
        BNTPMessage msg;
        
        memcpy(&msg.requestOrigination, compressed + byteOffset, sizeof(msg.requestOrigination));
        byteOffset += sizeof(msg.requestOrigination);

        memcpy(&msg.requestReceipt, compressed + byteOffset, sizeof(msg.requestReceipt));
        byteOffset += sizeof(msg.requestReceipt);

        memcpy(&msg.receiver, compressed + byteOffset, sizeof(msg.receiver));
        byteOffset += sizeof(msg.receiver);

        messages.push_back(msg);
    }

    return true;
}

size_t BNTPResponseComponent::getSize() {
    // size(n) + size([origination, reciept, reciever]) * n
    return 1 + 9 * messages.size();
}