/* Author : Sneha Ramaiah  Suraksha Yelawala Swamygowda 
 * Description: This file defines the two required functions for the branch predictor.
*/
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include "predictor.h"
#include <math.h>

//Tournament Predictor

//Create Table for local History
struct localHistoryBits* localHistoryTable;

//Create Table for Local Prediction and path History
struct localPredictBits* localPredict;

struct pHistory pathHistory;


//These tables are for the global and choosing predictors
char* globalPredictionTable;
char* choicePredictionTable;

//Perceptron Predictor
int PerTa[Per_Table_Entry][N] = { 0 };   // Perceptron Table
int bias[Per_Table_Entry] = { 0 };       // weight for constant number
int indexPerTa;                          // index into PT after hashing
int y;                                   // result of weight calculation
bool p_prediction;                       // result of prediction

//global history register
struct globalHistory globalHistory;


//Initializes Predictor 
void init_predictor()
{
#ifdef TOURNAMENT
    //Set Path History to 0
    pathHistory.bits = 0;

    //Allocating memory for Local, Global and Path history Tabel
    localHistoryTable     = new localHistoryBits[LOCAL_TABLE_SIZE]; 
    localPredict          = new localPredictBits[LOCAL_TABLE_SIZE];
    globalPredictionTable = new char[GLOBAL_TABLE_SIZE];
    choicePredictionTable = new char[GLOBAL_TABLE_SIZE];


    //Set all Local History and Predict table entries to 0(NOT_TAKEN)
    for (int i = 0; i < LOCAL_TABLE_SIZE; i++) {

        localHistoryTable[i].bits = 0;
        localPredict[i].bits = 0;
    }
    //Set all Global and Choice Prediction table entries to  WNT
    std::fill(globalPredictionTable, globalPredictionTable + GLOBAL_TABLE_SIZE, WNT);
    std::fill(choicePredictionTable, choicePredictionTable + GLOBAL_TABLE_SIZE, WNT);

#endif // TOURNAMENT

#ifdef PERCEPTRON
    globalHistory.bits = 0; //intitializing the vlaue of global history register to 0

#endif // PERCEPTRON

    
}

bool localPrediction(uint16_t pcIndex) {
    constexpr uint8_t TAKEN_THRESHOLD = 4;

    // local prediction table index
    uint16_t predictIndex = localHistoryTable[pcIndex].bits;

    // Determine if branch is taken or not based on the Local Prediction Table
    return (localPredict[predictIndex].bits >= TAKEN_THRESHOLD) ? TAKEN : NOT_TAKEN;

}

bool globalPrediction() {

    constexpr uint8_t TAKEN_THRESHOLD = 1;

    // path history bits for global history table
    uint32_t pIndex = pathHistory.bits;

    // Determine if branch is taken or not based on the Global Prediction Table
    return (globalPredictionTable[pIndex] <= TAKEN_THRESHOLD) ? NOT_TAKEN : TAKEN;
}


bool PREDICTOR::get_prediction(const branch_record_c* br, const op_state_c* os)
{
#ifdef TOURNAMENT
    bool p_prediction = TAKEN;

    if (br->is_conditional) {
        constexpr uint32_t BIT_MASK = 0x3FF; // Bit mask for extracting 10 bits
        uint16_t pcIndex = (br->instruction_addr >> 2) & BIT_MASK; // Extract 10 bits from Program Counter[11:2]

        // Determine whether to use global or local predictors based on the choice prediction
        bool useLocalPredictor = (choicePredictionTable[pathHistory.bits] <= 1);

        // If using local predictor, directly use the local prediction for the given PC index
        // Otherwise, use the global predictor
        p_prediction = (useLocalPredictor) ? localPrediction(pcIndex) : globalPrediction();
    }

#endif // TOURNAMENT

#ifdef PERCEPTRON
    if (br->is_conditional) {
        indexPerTa = ((br->instruction_addr >> 2) % Per_Table_Entry) ^ (globalHistory.bits % Per_Table_Entry); // hash pc into index xor the instruction address initial 2 bit is not considered  and globalhistory bits
        y = bias[indexPerTa]; // add bias to y = w0
        uint32_t mask = 1;
        // calculate correlation between current branch and history
        for (int i = 0; i < N; i++) {
            int x = (globalHistory.bits & mask) ? 1 : -1;//for getting vakue of x if it is taken 1 else -1
            y += PerTa[indexPerTa][i] * x; //using the formula y = summation from i to N xw
            mask <<= 1;
        }

        if (y > 0) p_prediction = TAKEN; //if the result of y  is greater than 0 taken else not taken
        else p_prediction = NOT_TAKEN;
    }
#endif // PERCEPTRON
    return p_prediction;
}


/*
 * This function updates the choice table
 */
void update_choicePredictor(uint16_t pcIndex, bool prediction) {  

    // Get predictions from local and global predictors
    bool localChoice = localPrediction(pcIndex);
    bool globalChoice = globalPrediction();

    // Calculate the index for the choice prediction table
    uint16_t pathHistoryIndex = pathHistory.bits;

    // Update the choice prediction table based on the correctness of predictions if localchoice is correct -1 assign if globalchoice is correct 1 orelse it remains 0
    int predictionDelta = (localChoice == prediction && globalChoice != prediction) ? -1 :
        ((localChoice != prediction && globalChoice == prediction) ? 1 : 0);

    // Ensure the choice prediction table index is within bounds
    pathHistoryIndex = pathHistoryIndex & 0xFFF;  // Assuming 12 bits for the path history index

    // Perform conditional increment/decrement based on prediction correctness if local predict correct and greater than zero decrement the counter or else increment the counter if less than 3
    if (predictionDelta < 0 && choicePredictionTable[pathHistoryIndex] > 0) {
        choicePredictionTable[pathHistoryIndex]--;
    }
    else if (predictionDelta > 0 && choicePredictionTable[pathHistoryIndex] < 3) {
        choicePredictionTable[pathHistoryIndex]++;
    }
}

/*
 * This function updates the local predictor table
 */
void update_localPredictor(uint16_t pcIndex, bool predict) {

    // Update Prediction Table
    uint16_t predictIndex = localHistoryTable[pcIndex].bits; // Gets the Index to be used in the Local Prediction table
    uint16_t prediction = localPredict[predictIndex].bits;    // Gets the prediction saturation count

    // If prediction is true, increment; decrement otherwise
    int predictionDelta = predict ? 1 : -1;
    if (predictionDelta > 0 && prediction < 7) {
        localPredict[predictIndex].bits++;
    }
    else if (predictionDelta < 0 && prediction > 0) {
        localPredict[predictIndex].bits--;
    }

    // Update Local History Shift bits to push last prediction
    localHistoryTable[pcIndex].bits = (localHistoryTable[pcIndex].bits << 1) | predict;
}

/*
 * This function updates the global predictor table
 */
void update_globalPredictor(bool predict) {

    // Update Prediction Table
    uint16_t prediction = globalPredictionTable[pathHistory.bits]; // Gets the prediction saturation count

    // If prediction is true then increment, decrement if otherwise
    int predictionDelta = predict ? 1 : -1;
    if (predictionDelta > 0 && prediction < 3) {
        globalPredictionTable[pathHistory.bits]++;
    }
    else if (predictionDelta < 0 && prediction > 0) {
        globalPredictionTable[pathHistory.bits]--;
    }

    // Update path History Shift bits to push last prediction
    pathHistory.bits = (pathHistory.bits << 1) | predict;

}




// Update the predictor after a prediction has been made.  This should accept
// the branch record (br) and architectural state (os), as well as a third
// argument (taken) indicating whether or not the branch was taken.
void PREDICTOR::update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
{
#ifdef TOURNAMENT
    /* replace this code with your own */

    uint32_t bitMask = 0x3FF;
    uint16_t pcIndex = (br->instruction_addr >> 2) & bitMask; //Extract the  10 bits of Program Counter[11:2] for indexing

    if (br->is_conditional) {
        //Update Predictors and History Tables
        update_choicePredictor(pcIndex, taken);
        update_localPredictor(pcIndex, taken);
    }
        update_globalPredictor(taken);
#endif // TOURNAMENT




#ifdef PERCEPTRON
        if (br->is_conditional) {
            indexPerTa = ((br->instruction_addr >> 2) % Per_Table_Entry) ^ (globalHistory.bits % Per_Table_Entry); // hash pc into index xor the instruction address initial 2 bit is not considered  and globalhistory bits
            if ((std::abs(y) <= theta) || (p_prediction != taken)) { //if absolute value of y is less than or equal theta or actual and presiction not equal

                if (bias[indexPerTa] != Max_Weight && taken) { //if the bias is not equal to 128 or -128
                    bias[indexPerTa] +=1; //increment if it is taken or decrement if it is not taken of bias
                }
                else if (bias[indexPerTa] != Min_Weight && !taken) { //if the bias is not equal to 128 or -128
                    bias[indexPerTa] += -1; //increment if it is taken or decrement if it is not taken of bias
                }


                // Updating weights
                uint32_t ghrMask = 1;
                for (int i = 0; i < N; i++) {
                    int x = ((globalHistory.bits & ghrMask)) ? 1 : -1; //getting value of x
 
                    if (PerTa[indexPerTa][i] != Max_Weight && taken) { //if the bias is not equal to 128 or -128
                        PerTa[indexPerTa][i] += (x * 1); //new wi = wi + x * t
                    }
                    else if (PerTa[indexPerTa][i] != Min_Weight && !taken) { //if the bias is not equal to 128 or -128
                        PerTa[indexPerTa][i] += (x *  (- 1)); //new wi = wi + x * t
                    }
                    ghrMask <<= 1; //to get all the bits value
                }
            }
            globalHistory.bits = (globalHistory.bits << 1) | taken; //to update the global history table with the taken value
        }
#endif // PERCEPTRON

    
    
}


void cleanup() {
    delete localHistoryTable;
    delete localPredict;
    delete globalPredictionTable;
    delete choicePredictionTable;
}

