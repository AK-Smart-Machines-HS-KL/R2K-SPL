/**
 * @file R2KAudioProvider.cpp
 * This file implements a module that uses tflite.
 * @author Thomas Jäger
 */

#include "R2KAudioProvider.h"
#include "Platform/File.h"

MAKE_MODULE(R2KAudioProvider, infrastructure);

// std::mutex R2KAudioProvider::mutex;

#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x))                                                  \
  {                                                          \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }

R2KAudioProvider::R2KAudioProvider()
{
  // Load model
  //std::string filepath = std::string(File::getBHDir()) + "/Config/NeuralNets/mobilenet_v1_1.0_224_quant.tflite";
  //std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(filepath.c_str());
  //TFLITE_MINIMAL_CHECK(model != nullptr);
  //printf("tflite model path %s.\n", filepath.headName.c_str());
  // Build the interpreter with the InterpreterBuilder.
  // Note: all Interpreters should be built with the InterpreterBuilder,
  // which allocates memory for the Interpreter and does various set up
  // tasks so that the Interpreter can read the provided model.
  //tflite::ops::builtin::BuiltinOpResolver resolver;
}

void R2KAudioProvider::update(R2KAudioData &r2kAudioData)
{
  // Load model
  std::string filepath = std::string(File::getBHDir()) + "/Config/NeuralNets/mobilenet_v1_1.0_224_quant.tflite";
  std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(filepath.c_str());
  TFLITE_MINIMAL_CHECK(model != nullptr);

  // Build the interpreter with the InterpreterBuilder.
  // Note: all Interpreters should be built with the InterpreterBuilder,
  // which allocates memory for the Interpreter and does various set up
  // tasks so that the Interpreter can read the provided model.
  
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model, resolver);
  std::unique_ptr<tflite::Interpreter> interpreter;
  builder(&interpreter);
  TFLITE_MINIMAL_CHECK(interpreter != nullptr);

  // Allocate tensor buffers.
  TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
  tflite::PrintInterpreterState(interpreter.get());

  // Run inference
  TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
  tflite::PrintInterpreterState(interpreter.get());

  r2kAudioData.dummyData = 1;
}
