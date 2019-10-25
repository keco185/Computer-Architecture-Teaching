#include "Branch_Predictor.h"

const unsigned instShiftAmt = 2; // Number of bits to shift a PC by

// You can play around with these settings.
const unsigned localPredictorSize = 2048;   // two-bit
const unsigned localCounterBits = 2;        //two-bit
const unsigned localHistoryTableSize = 2048;// Tournament
const unsigned globalPredictorSize = 32768; // Tournament & gshare
const unsigned globalCounterBits = 64;       // Tournament, gshare, Perceptron, 12-64, also update main.c for recod keeping
const unsigned choicePredictorSize = 8192;  // Tournament Keep this the same as globalPredictorSize.
const unsigned choiceCounterBits = 2;       // Tournament ~
const unsigned perceptronSize = 32768;

Branch_Predictor *initBranchPredictor()
{
    Branch_Predictor *branch_predictor = (Branch_Predictor *)malloc(sizeof(Branch_Predictor));
	
   // Perceptron
	
	branch_predictor->global_predictor_size = globalPredictorSize;
    branch_predictor->global_history_mask = globalPredictorSize - 1;
    branch_predictor->global_history = 0; // global history register
    
	// Initialize global counters
    branch_predictor->global_counters = (Sat_Counter *)malloc(globalPredictorSize * sizeof(Sat_Counter));
    
	for (int i = 0; i < globalPredictorSize; i++)
	{
        initSatCounter(&(branch_predictor->global_counters[i]), globalCounterBits);
	}
	
    assert(checkPowerofTwo(perceptronSize));

    branch_predictor -> perceptron_size = perceptronSize;

    // Initialize threshold for branch prediction
    branch_predictor -> threshold =1.93 * globalCounterBits + 14; // best threshold given history length of h (found on page 201)

    unsigned perceptronBit = 1 + floor(log2(branch_predictor -> threshold));

    branch_predictor -> perceptron_mask = perceptronSize - 1;

    branch_predictor -> perceptron = (Perceptron *)malloc(perceptronSize * sizeof(Perceptron));

    for (int i = 0; i < perceptronSize; i++)
    {
        initPerceptron(&(branch_predictor->perceptron[i]), globalCounterBits);
    }

    return branch_predictor;
}

// sat counter functions
inline void initSatCounter(Sat_Counter *sat_counter, unsigned counter_bits)
{
    sat_counter->counter_bits = counter_bits;
    sat_counter->counter = 0;
    sat_counter->max_val = (1 << counter_bits) - 1;
}

inline void incrementCounter(Sat_Counter *sat_counter)
{
    if (sat_counter->counter < sat_counter->max_val)
    {
        ++sat_counter->counter;
    }
}

inline void decrementCounter(Sat_Counter *sat_counter)
{
    if (sat_counter->counter > 0) 
    {
        --sat_counter->counter;
    }
}

// Branch Predictor functions
bool predict(Branch_Predictor *branch_predictor, Instruction *instr)
{
    uint64_t branch_address = instr -> PC;
	
    // Step one, get prediction
    unsigned perceptron_idx = (branch_predictor->global_history & branch_predictor->global_history_mask) ^ (branch_address & branch_predictor->global_history_mask);

    int64_t y = computePerceptron(&(branch_predictor -> perceptron[perceptron_idx]), &(branch_predictor -> global_counters[perceptron_idx]));
    train(&(branch_predictor -> perceptron[perceptron_idx]), branch_predictor -> threshold, &(branch_predictor -> global_counters[perceptron_idx]), instr -> taken, y);

    bool prediction = (y > 0);
    
    return prediction == instr -> taken;
}

// Perceptron
inline void initPerceptron(Perceptron *perceptron, unsigned counter_bits)
{
	perceptron -> weight = (int64_t *)malloc(counter_bits * sizeof(int64_t));
	
	for(int i = 0; i <= counter_bits; i++)
	perceptron -> weight[i] = 1;    
        perceptron -> num_perceptron = counter_bits;
}

inline int64_t computePerceptron(Perceptron *perceptron, Sat_Counter *sat_counter)
{
	int64_t y = perceptron -> weight[0];
	for(int i =1; i <= sat_counter -> counter_bits; i++)
	{
		int8_t temp = (sat_counter -> counter & (1 << i));
		int8_t x = 1;
		
		if(temp < 0)
			x= -1;
		y+= x *perceptron -> weight[i];
	}
	return y;
}

inline void train( Perceptron *perceptron, unsigned threshold, Sat_Counter *global, bool is_taken, int64_t y)
{
	if((y < 0) == is_taken || (y > 0) == is_taken || y <= threshold)
		
		for(int i = 0; i <= global -> counter_bits; i++)
	{
		int8_t temp = (global -> counter & (1 << i));
		int8_t x = 1;
		int8_t t = 1;
		
	        if(temp < 0)
			x = -1;
		
		if(is_taken == false)
			t = -1;
		perceptron -> weight[i] = perceptron -> weight[i] + (t * x);
	}
}

inline unsigned getIndex(uint64_t branch_addr, unsigned index_mask)
{
    return (branch_addr >> instShiftAmt) & index_mask;
}

inline bool getPrediction(Sat_Counter *sat_counter)
{
    uint8_t counter = sat_counter->counter;
    unsigned counter_bits = sat_counter->counter_bits;

    // MSB determins the direction
    return (counter >> (counter_bits - 1));
}

int checkPowerofTwo(unsigned x)
{
    //checks whether a number is zero or not
    if (x == 0)
    {
        return 0;
    }

    //true till x is not equal to 1
    while( x != 1)
    {
        //checks whether a number is divisible by 2
        if(x % 2 != 0)
        {
            return 0;
        }
        x /= 2;
    }
    return 1;
}
