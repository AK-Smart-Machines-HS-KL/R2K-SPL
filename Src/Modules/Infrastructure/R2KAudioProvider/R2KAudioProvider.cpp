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
  printf("=== Pre-invoke Interpreter State ===\n");
  tflite::PrintInterpreterState(interpreter.get());

  // Fill input buffers
  int input_tensor_index = 0; // Assuming the first tensor is the input
  TfLiteTensor* input_tensor = interpreter->tensor(interpreter->inputs()[input_tensor_index]);
  int total_input_elements = 1;
  for (int i = 0; i < input_tensor->dims->size; i++) {
      total_input_elements *= input_tensor->dims->data[i];
  }

  float* input_data_ptr = interpreter->typed_input_tensor<float>(input_tensor_index);
  for (int i = 0; i < total_input_elements; i++) {
      input_data_ptr[i] = 1.0f; // Fill with dummy values
  }

  // Run inference
  TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
  printf("\n\n=== Post-invoke Interpreter State ===\n");
  tflite::PrintInterpreterState(interpreter.get());

  // Read output buffers
  int output_tensor_index = 0; // Assuming the first tensor is the output
  TfLiteTensor* output_tensor = interpreter->tensor(interpreter->outputs()[output_tensor_index]);
  int total_output_elements = 1;
  for (int i = 0; i < output_tensor->dims->size; i++) {
      total_output_elements *= output_tensor->dims->data[i];
  }

  float* output_data_ptr = interpreter->typed_output_tensor<float>(output_tensor_index);
  printf("\nOutput tensor values:\n");
  for (int i = 0; i < total_output_elements; i++) {
      printf("%f ", output_data_ptr[i]);
  }
  printf("\n");

  r2kAudioData.dummyData = 1;
}
