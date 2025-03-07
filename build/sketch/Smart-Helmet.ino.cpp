#include <Arduino.h>
#line 1 "E:\\COMPETITIONS\\REPOS\\Smart-Helmet\\Smart-Helmet.ino"
#include <TensorFlowLite_ESP32.h>
#include "model_data.cc"  // Include the converted model as a C array
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/schema/schema_generated.h>

// Define the tensor arena size
const int tensor_arena_size = 2 * 1024;  // 2 KB memory for the model
uint8_t tensor_arena[tensor_arena_size];

// TensorFlow Lite Micro objects
tflite::MicroInterpreter* interpreter;
TfLiteTensor* input;
TfLiteTensor* output;

// Make sure model_data is properly defined - add this if it's not in model_data.cc
extern const unsigned char model_data[];

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 TensorFlow Lite Micro Model Inference");

    // Load the model
    const tflite::Model* model = tflite::GetModel(model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("Model schema version is not compatible!");
        return;
    }

    // Define operations resolver and interpreter
    static tflite::MicroMutableOpResolver<5> resolver;
    resolver.AddFullyConnected();
    resolver.AddRelu();

    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, tensor_arena_size, nullptr, nullptr, nullptr);
    
    interpreter = &static_interpreter;

    // Allocate memory for the model's tensors
    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("Tensor allocation failed");
        return;
    }

    // Get input and output tensors
    input = interpreter->input(0);
    output = interpreter->output(0);
}

void loop() {
    // Example input data (replace with actual sensor data)
    float input_data[3] = {1.0, 1.1, 0.9};

    // Fill the input tensor
    for (int i = 0; i < 3; i++) {
        input->data.f[i] = input_data[i];
    }

    // Run inference
    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("Model invocation failed");
        return;
    }

    // Read output and display the result
    Serial.print("Model output: ");
    for (int i = 0; i < 3; i++) {
        Serial.print(output->data.f[i]);
        Serial.print(" ");
    }
    Serial.println();

    delay(1000);
}

