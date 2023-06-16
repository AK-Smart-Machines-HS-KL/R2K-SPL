#include "BNTPResponse.h"
#include <limits.h>
#include "Platform/BHAssert.h"

// n(1), [origination(4), reciept(4), reciever(1)] x n

size_t BNTPResponseComponent::compress(char* buff) {
    size_t byteOffset = 0;
    
    // size must be less than max(n)
    ASSERT(messages.size() < std::numeric_limits<uint8_t>::max());
    uint8_t n = (uint8_t) messages.size();

    // n first
    memcpy(buff, &n, sizeof(n)); 
    byteOffset += sizeof(n);

    for (auto &msg : messages)
    {
        memcpy(buff, &msg.requestOrigination, sizeof(msg.requestOrigination)); 
        byteOffset += sizeof(msg.requestOrigination);

        memcpy(buff, &msg.requestReceipt, sizeof(msg.requestReceipt)); 
        byteOffset += sizeof(msg.requestReceipt);

        memcpy(buff, &msg.receiver, sizeof(msg.receiver)); 
        byteOffset += sizeof(msg.receiver);
    }

    return byteOffset;
}

bool BNTPResponseComponent::decompress(char* compressed) {
    size_t byteOffset = 0;
    int n;

    messages.clear();

    memcpy(&n, compressed, sizeof(n)); 
    byteOffset += sizeof(n);

    for (size_t i = 0; i < n; i++)
    {
        BNTPMessage msg;
        
        memcpy(&msg.requestOrigination, compressed, sizeof(msg.requestOrigination)); 
        byteOffset += sizeof(msg.requestOrigination);

        memcpy(&msg.requestReceipt, compressed, sizeof(msg.requestReceipt)); 
        byteOffset += sizeof(msg.requestReceipt);

        memcpy(&msg.receiver, compressed, sizeof(msg.receiver)); 
        byteOffset += sizeof(msg.receiver);

        messages.push_back(msg);
    }

    return true;
}

size_t BNTPResponseComponent::getSize() {
    // size(n) + size([origination, reciept, reciever]) * n
    return 1 + 9 * messages.size();
}