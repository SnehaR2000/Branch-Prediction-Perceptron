/* Author:Sneha Ramaiah  Suraksha Yelawala Swamygowda 
 *
 * C version of predictor file
*/

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <cstddef>
#include <cstring>
#include <inttypes.h>
#include <vector>
#include "op_state.h"   // defines op_state_c (architectural state) class 
#include "tread.h"      // defines branch_record_c class

//Define Status of Prediction
#define  NOT_TAKEN 0
#define  TAKEN     1


// Tournamnet Predictor 

//Define Saturation Counter for Tournament predictor
#define SNT 0
#define WNT 1
#define WT  2
#define ST  3

//Defining the  sizes for local and global predictors
constexpr int LOCAL_TABLE_SIZE  =  1024;
constexpr int GLOBAL_TABLE_SIZE =  4096;

//Define Structure for localHistoryBits 10 bits
struct localHistoryBits {
    uint16_t bits : 10;
};

//Define Structure for localPredictBits 3 bits
struct localPredictBits {
    uint8_t bits : 3;
};

//Define Structure for pathHistory 12 bits
struct pHistory {
    unsigned bits : 12;
};

//Perceptron Predictor

constexpr int N               = 28;                    // length of global history table
constexpr int Space           = (4 * 1024);            // space budget is 4K 
constexpr int theta           = ((N * 1.93) + 14);     // threshold to calculate
constexpr int weight_bits     = 8;                     // No. bit for  each weight
constexpr int Max_Weight      = 128;                   // Max weight postive value it can hold
constexpr int Min_Weight      = -128;                  // Min weight postive value it can hold
constexpr int Per_Table_Entry = 128;                   //Total Number of Entries to Perceptron Prediction Table

//Space Budget 
//No. of Table of Perceptrons per bit  = 128
//No. of Bits in global history register = 28 bits
//No. of weight_bits = 8 bits 
// total size of bias table = 8*128 =1024
//Therefore Total size of perceptron table  = (128 * 28 * 8) + 1024  = 29696 bits
//TOTAL BUDGET = Total size of perceptron table + No. of Bits in global history register = 29696 + 28 = 29724 bits = 3.71K

//global history register
struct globalHistory {
    unsigned bits : 28;
};


class PREDICTOR
{
public:
    bool get_prediction(const branch_record_c* br, const op_state_c* os);
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken);

};


/*
  Initialize the predictor.
*/
void init_predictor();

/*
  local Predicrion.
*/

bool localPrediction(uint16_t pcIndex);

/*
  global Prediction
*/
bool globalPrediction();
/*
  Free up the Memory Space 
*/
void cleanup();

#endif // PREDICTOR_H_SEEN








