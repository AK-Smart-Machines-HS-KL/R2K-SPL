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

    memcpy(compressed + byteOffset, &x, sizeof(float));
    byteOffset += sizeof(float);

    memcpy(compressed + byteOffset, &y, sizeof(float));
    byteOffset += sizeof(float);

    pose.translation.x() = x;
    pose.translation.y() = y;

    memcpy(compressed + byteOffset, &pose.rotation, sizeof(Angle));
    byteOffset += sizeof(pose.rotation);
    return byteOffset == getSize();
}

size_t RobotPoseComponent::getSize() {
    return 12;
}