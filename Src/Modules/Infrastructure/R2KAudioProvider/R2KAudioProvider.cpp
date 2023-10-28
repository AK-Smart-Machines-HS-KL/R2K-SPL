/**
 * @file R2KAudioProvider.cpp
 * This file implements a module that uses tflite.
 * @author Thomas Jäger
 */

#include "R2KAudioProvider.h"
#include "Platform/File.h"

MAKE_MODULE(R2KAudioProvider, infrastructure);

#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x))                                                  \
  {                                                          \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }

R2KAudioProvider::R2KAudioProvider()
{
    // Load model
  std::string filepath = std::string(File::getBHDir()) + "/Config/NeuralNets/mobilenet_v1_1.0_224_quant.tflite";
  model = tflite::FlatBufferModel::BuildFromFile(filepath.c_str());
  TFLITE_MINIMAL_CHECK(model != nullptr);

  // Build the interpreter with the InterpreterBuilder.
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model, resolver);
  builder(&interpreter);
  TFLITE_MINIMAL_CHECK(interpreter != nullptr);

  // Allocate tensor buffers.
  TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
  tflite::PrintInterpreterState(interpreter.get());
}

void R2KAudioProvider::update(R2KAudioData &r2kAudioData)
{
  if (!interpreter) // Ensure interpreter is loaded
      return;

  printf("=== fill input tensor ===\n");
  // Fill input buffers
  float* input = interpreter->typed_input_tensor<float>(0);
  // Here you should fill the input buffer appropriately
  // For this example, I'm assuming you fill with some dummy value.
  *input = 0.0f;
  
  printf("=== RDY fill input tensor ===\n");

  // Run inference
  printf("\n\n=== Running Inference ===\n");
  TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
  tflite::PrintInterpreterState(interpreter.get());
  printf("\n\n=== Inference Done!! ===\n");
  
  /*
  TfLiteTensor* output_tensor = interpreter->output_tensor(0);
  if (output_tensor->type != kTfLiteFloat32) {
      fprintf(stderr, "Expected output tensor to be of type float!\n");
      return;
  }
  */

  // Read output buffers
  printf("\n\n=== Reading output ===\n");
  uint8_t* output = interpreter->typed_output_tensor<uint8_t>(0);
  if(output != nullptr) {
    float output_float = static_cast<float>(*output);  // Convert to float if necessary
    printf("Output = %f\n", output_float);
    r2kAudioData.dummyData = output_float; 
  }
  printf("\n\n=== Reading output done ===\n");

}
