#ifndef __BRANCH_PREDICTOR_HH__
#define __BRANCH_PREDICTOR_HH__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "Instruction.h"

// Predictor type
//#define TWO_BIT_LOCAL
//#define TOURNAMENT
//#define GSHARE
#define PERCEPTRON

// saturating counter
typedef struct Sat_Counter
{
    unsigned counter_bits;
    // uint8_t max_val;
    // uint8_t counter;
    uint64_t max_val;
    uint64_t counter;
}Sat_Counter;

typedef struct Perceptron
{
    int64_t *weight;
    unsigned num_perceptron;
} Perceptron;

typedef struct Branch_Predictor
{
	unsigned global_predictor_size;
    unsigned global_history_mask;
    Sat_Counter *global_counters;
    uint64_t global_history;
	
	
    unsigned perceptron_mask;
    unsigned perceptron_size;
    unsigned threshold;
    Perceptron *perceptron;
    
} Branch_Predictor;

// Initialization function
Branch_Predictor *initBranchPredictor();

// Counter functions
void initSatCounter(Sat_Counter *sat_counter, unsigned counter_bits);
void incrementCounter(Sat_Counter *sat_counter);
void decrementCounter(Sat_Counter *sat_counter);

// Branch predictor functions
bool predict(Branch_Predictor *branch_predictor, Instruction *instr);

// Perceptron
void initPerceptron(Perceptron *perceptron, unsigned counter_bits);
int64_t computePerceptron(Perceptron *perceptron, Sat_Counter *sat_counter);
void train(Perceptron *perceptron, unsigned threshold, Sat_Counter * sat_counter, bool is_taken, int64_t y);

unsigned getIndex(uint64_t branch_addr, unsigned index_mask);
bool getPrediction(Sat_Counter *sat_counter);

// Utility
int checkPowerofTwo(unsigned x);

#endif
