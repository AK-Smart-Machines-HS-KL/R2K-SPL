#include "BallModel.h"
// #include "Representations/Modeling/BallModel.h"
#include <cstdint>

/*
* 
* struct Vector2 {
    int16_t x; // 14bit
    int16_t y; // 14bit

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

  // last_perception
  float lx = model.lastPerception.x();
  memcpy(buff + byteOffset, &lx, sizeof(float));
  byteOffset += sizeof(float);

  float ly = model.lastPerception.y();
  memcpy(buff + byteOffset, &ly, sizeof(float));
  byteOffset += sizeof(float);


  // estimate
  float vx = model.estimate.velocity.x(); // 14bit
  memcpy(buff + byteOffset, &vx, sizeof(float));
  byteOffset += sizeof(float);

  float vy = model.estimate.velocity.y(); // 14bit
  memcpy(buff + byteOffset, &vy, sizeof(float));
  byteOffset += sizeof(float);


  //time
  unsigned int t = model.timeWhenLastSeen;
  memcpy(buff + byteOffset, &t, sizeof(unsigned int));
  byteOffset += sizeof(unsigned int);

  unsigned int t2 = model.timeWhenDisappeared;
  memcpy(buff + byteOffset, &t2, sizeof(unsigned int));
  byteOffset += sizeof(unsigned int);

  // struct BallState estimate; // 56bit
  // BallState s = model.estimate;
  float x = model.estimate.position.x(); // 14bit
  memcpy(buff + byteOffset, &x, sizeof(float));
  byteOffset += sizeof(float);

  float y = model.estimate.position.y() ; // 14bit
  memcpy(buff + byteOffset, &y, sizeof(float));
  byteOffset += sizeof(float);

  // uint8_t seen_percentage; 
  uint8_t sp = model.seenPercentage;
  memcpy(buff + byteOffset, &sp, sizeof(sp));
  byteOffset += sizeof(sp);

    // return getSize();
  return byteOffset;
}

bool BallModelComponent::decompress(uint8_t* compressed) {
  size_t byteOffset = 0;

  // last_perception
  float lx;
  memcpy(&lx, compressed + byteOffset, sizeof(lx));
  byteOffset += sizeof(lx);
  model.lastPerception.x() = lx;

  float ly;
  memcpy(&lx, compressed + byteOffset, sizeof(ly));
  byteOffset += sizeof(ly);
  model.lastPerception.y() = ly;

  // estimate
  float vx;
  memcpy(&vx, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(vx);
  model.estimate.velocity.x() = vx;

  float vy;
  memcpy(&vy, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(vy);
  model.estimate.velocity.y() = vy;

  //time 
  unsigned int t;
  memcpy(&t, compressed + byteOffset, sizeof(unsigned int));
  byteOffset += sizeof(t);
  model.timeWhenLastSeen = t;

  unsigned int t2;
  memcpy(&t2, compressed + byteOffset, sizeof(unsigned int));
  byteOffset += sizeof(t2);
  model.timeWhenDisappeared = t2;

  float x;
  memcpy(&x, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(x);
  model.estimate.position.x() = x;

  float y;
  memcpy(&y, compressed + byteOffset, sizeof(float));
  byteOffset += sizeof(y);
  model.estimate.position.y() = y;

  // uint8_t seen_percentage; 
  uint8_t sp;
  memcpy(&sp, compressed + byteOffset, sizeof(sp));
  byteOffset += sizeof(sp);
  model.seenPercentage = sp;

  return byteOffset == getSize();
}

size_t BallModelComponent::getSize() {
    return  6*sizeof(float) + 2*sizeof(unsigned int) + sizeof(uint8_t);
}