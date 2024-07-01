#include "BallModel.h"
// #include "Representations/Modeling/BallModel.h"
#include <cstdint>

/*
      Number of bytes to encode struct BallState
      #define BYTES_LENGTH_BALL_STATE 7

      struct BallState {
        struct Vector2 position; // 28bit
        struct Vector2 velocity; // 28bit
      };

      // Number of bytes to encode struct BallModel
      #define BYTES_LENGTH_BALL_MODEL 28

      struct BallModel {
        struct Vector2 last_perception; // 28bit
        struct BallState estimate; // 56bit
        Timestamp time_when_last_seen; // 64bit
        Timestamp time_when_disappeared; // 64bit
        uint8_t seen_percentage; // 7bit
      };
*/ 

size_t BallModelComponent::compress(uint8_t* buff) {
  size_t byteOffset = 0;

  unsigned int t = model.timeWhenLastSeen;
  memcpy(buff + byteOffset, &t, sizeof(unsigned int));
  byteOffset += sizeof(unsigned int);


  // struct BallState estimate; // 56bit
  // BallState s = model.estimate;
  float x = model.estimate.position.x(); // 14bit
  memcpy(buff + byteOffset, &x, sizeof(float));
  byteOffset += sizeof(float);

  float y = model.estimate.position.y() ; // 14bit
  memcpy(buff + byteOffset, &y, sizeof(float));
  byteOffset += sizeof(float);

  float vx = model.estimate.velocity.x(); // 14bit
  memcpy(buff + byteOffset, &vx, sizeof(float));
  byteOffset += sizeof(float);

  float vy = model.estimate.velocity.y(); // 14bit
  memcpy(buff + byteOffset, &vy, sizeof(float));
  byteOffset += sizeof(float);

    // return getSize();
  return byteOffset;
}

bool BallModelComponent::decompress(uint8_t* compressed) {
  size_t byteOffset = 0;

  unsigned int t;
  memcpy(&t, compressed + byteOffset, sizeof(unsigned int));
  byteOffset += sizeof(t);
  model.timeWhenLastSeen = t;

  float x;
  memcpy(&x, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(t);
  model.estimate.position.x() = x;

  float y;
  memcpy(&y, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(t);
  model.estimate.position.y() = y;

  float vx;
  memcpy(&vx, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(t);
  model.estimate.velocity.x() = vx;

  float vy;
  memcpy(&vy, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(t);
  model.estimate.velocity.y() = vy;

  return byteOffset == getSize();
}

size_t BallModelComponent::getSize() {
    return sizeof(unsigned int) + 4 * sizeof(float);
}