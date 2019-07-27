// Runtime files
#include "LSTM.h"
#include "misc.h"

// Other includes
#include <iostream>
#include <getopt.h>
#include <stdlib.h>
#include <vector>
#include <cassert>
#include <cstdio>
#include <cuda_profiler_api.h>


int main(int argc, char** argv) {

  uint32_t input = 256;
  uint32_t hidden = 256;
  uint32_t batch = 40;
  uint32_t x_tile_size = 2;
  uint32_t y_tile_size = 4;
  uint32_t num_groups = 64;
  uint32_t reduction_width = 8;
  uint32_t input_length = 100;

  std::vector<float *> weights;
  /* create LSTM weights including: 
  W [input x hidden] x4
  U [hidden x hidden] x4
  b [hidden] x4
  */
  create_dummy_weights_lstm(weights, input, hidden);
 
  // Create layer
  LSTMLayer<float> layer = LSTMLayer<float>(input, hidden, batch, weights);
  
  // Declare model based on layer
  LSTMModel<float> model = LSTMModel<float>( {layer} );

  model.set_configuration(x_tile_size, y_tile_size, num_groups, reduction_width);
  model.initialize();
  
  float * testInput;
  /* pinned memory 
  input shape [batch x input x len]
  */
  cudaHostAlloc((void **) &testInput, sizeof(float) * batch * input * input_length, cudaHostAllocDefault); CUDA_ERR;

  /* init */
  for (uint32_t i = 0; i < batch * input * input_length; i++) {
    testInput[i] = (i / input) % batch;
  }

#ifdef DEBUG
  float temp = model.run_input(testInput, &input_length);
#else
  float time = 0.0f;
  for (int i = 0; i < 1000; i++) {
    float temp = model.run_input(testInput, &input_length);
  }
  cudaProfilerStart();
  for (int i = 0; i < 1000; i++) {
    float run_time = model.run_input(testInput, &input_length);
    time += run_time;
  }
  cudaProfilerStop();
  std::cout << time / 1000 << " ms\n";
#endif

  return 0;
}

