#include "RobotPose.h"


size_t RobotPoseComponent::compress(uint8_t* buff) {
    size_t byteOffset = 0;
    
    float x = pose.translation.x();
    float y = pose.translation.y();

    memcpy(buff + byteOffset, &x, sizeof(float));
    byteOffset += sizeof(float);

    memcpy(buff + byteOffset, &y, sizeof(float));
    byteOffset += sizeof(float);

    memcpy(buff + byteOffset, &pose.rotation, sizeof(Angle));
    byteOffset += sizeof(pose.rotation);
    return byteOffset;
}

bool RobotPoseComponent::decompress(uint8_t* compressed) {
    size_t byteOffset = 0;
    
    float x;
    float y;

    memcpy(&x, compressed + byteOffset, sizeof(float));
    byteOffset += sizeof(x); 

    memcpy(&y, compressed + byteOffset, sizeof(float));
    byteOffset += sizeof(y);

    pose.translation.x() = x;
    pose.translation.y() = y;

    memcpy(&pose.rotation, compressed + byteOffset, sizeof(Angle));
    byteOffset += sizeof(pose.rotation);
    return byteOffset == getSize();
}

size_t RobotPoseComponent::getSize() {
    return 12;
}