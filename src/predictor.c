//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// Student Information
//
const char *studentName = "Andrew Nathenson";
const char *studentID   = "A59012839";
const char *email       = "anathenson@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

//define number of bits required for indexing the BHT here. 
int ghistoryBits = 14; // Number of bits used for Global History
int bpType;       // Branch Prediction Type
int verbose;


//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//gshare
uint8_t *bht_gshare;
uint64_t ghistory;

// tournament
uint8_t *local_bht_tournament;
uint16_t *local_address_table;
uint8_t *global_bht_tournament;
uint8_t *chooser_bht_tournament;
uint64_t thistory;
int local_tournament_bits = 10;
int global_tournament_bits = 12;


// custom predictor: Skew predictor
int skewHistoryBits = 12;
uint8_t *skew_bht_1;
uint8_t *skew_bht_2;
uint8_t *skew_bht_3;
uint64_t shistory;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

//gshare functions
void init_gshare() {
 int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}



uint8_t 
gshare_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch(bht_gshare[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_gshare(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  //Update state of entry in bht based on outcome
  switch(bht_gshare[index]){
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void
cleanup_gshare() {
  free(bht_gshare);
}

////////////////////////////////////////////////

//tournament functions
void init_tournament() {
 int local_bht_entries = 1 << local_tournament_bits;
  local_bht_tournament = (uint8_t*)malloc(local_bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< local_bht_entries; i++){
    local_bht_tournament[i] = WN;
  }

  local_address_table = (uint16_t*)malloc(local_bht_entries * sizeof(uint16_t));
  for(i=0; i<local_bht_entries; i++){
    local_address_table[i] = 0;
  }

  int global_bht_entries = 1 << global_tournament_bits;
  global_bht_tournament = (uint8_t*)malloc(global_bht_entries * sizeof(uint8_t));
  for(i = 0; i< global_bht_entries; i++){
    global_bht_tournament[i] = WN;
  }

  chooser_bht_tournament = (uint8_t*)malloc(global_bht_entries * sizeof(uint8_t));
  for(i = 0; i< global_bht_entries; i++){
    chooser_bht_tournament[i] = WN;
  }


  thistory = 0;
}



uint8_t 
tournament_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t global_bht_entries = 1 << global_tournament_bits;
  uint32_t local_bht_entries = 1 << local_tournament_bits;
  uint32_t pc_lower_bits = pc & (local_bht_entries-1);
  uint32_t ghistory_lower_bits = thistory & (global_bht_entries -1);

  uint8_t use_local;

  switch(chooser_bht_tournament[ghistory_lower_bits]){
    case WN:
      use_local = NOTTAKEN;
      break;
    case SN:
      use_local = NOTTAKEN;
      break;
    case WT:
      use_local = TAKEN;
      break;
    case ST:
      use_local = TAKEN;
      break;
    default:
      printf("Warning: Undefined state of entry in chooser!\n");
      return NOTTAKEN;
      break;
  }

  if (use_local > 0) {
    switch(local_bht_tournament[local_address_table[pc_lower_bits]]){
      case WN:
        return NOTTAKEN;
        break;
      case SN:
        return NOTTAKEN;
        break;
      case WT:
        return TAKEN;
        break;
      case ST:
        return TAKEN;
        break;
      default:
        printf("Warning: Undefined state of entry in tournament local history!\n");
        return NOTTAKEN;
        break;
    }
  } else {
    switch(global_bht_tournament[ghistory_lower_bits]){
      case WN:
        return NOTTAKEN;
        break;
      case SN:
        return NOTTAKEN;
        break;
      case WT:
        return TAKEN;
        break;
      case ST:
        return TAKEN;
        break;
      default:
        printf("Warning: Undefined state of entry in tournament global history!\n");
        return NOTTAKEN;
        break;
    }
  }
}

void
train_tournament(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t global_bht_entries = 1 << global_tournament_bits;
  uint32_t local_bht_entries = 1 << local_tournament_bits;
  uint32_t pc_lower_bits = pc & (local_bht_entries-1);
  uint32_t ghistory_lower_bits = thistory & (global_bht_entries -1);

  uint8_t local_pred;
  // get result of local prediction to compare for chooser
  switch(local_bht_tournament[local_address_table[pc_lower_bits]]){
      case WN:
        local_pred = NOTTAKEN;
        break;
      case SN:
        local_pred = NOTTAKEN;
        break;
      case WT:
        local_pred = TAKEN;
        break;
      case ST:
        local_pred = TAKEN;
        break;
      default:
        printf("Warning: Undefined state of entry in tournament local history!\n");
        local_pred = NOTTAKEN;
        break;
    }
  
  uint8_t global_pred;
  // get result of global prediction to compare for chooser 
  switch(global_bht_tournament[ghistory_lower_bits]){
      case WN:
        global_pred = NOTTAKEN;
        break;
      case SN:
        global_pred = NOTTAKEN;
        break;
      case WT:
        global_pred = TAKEN;
        break;
      case ST:
        global_pred = TAKEN;
        break;
      default:
        printf("Warning: Undefined state of entry in tournament global history!\n");
        global_pred = NOTTAKEN;
        break;
    }

  uint32_t index = ghistory_lower_bits;
  //Update state of entry in global bht based on outcome
  switch(global_bht_tournament[index]){
    case WN:
      global_bht_tournament[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      global_bht_tournament[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      global_bht_tournament[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      global_bht_tournament[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in Tournament Global BHT!\n");
  }

  //Update local address table
  // The last & cuts off the leading digit so it is the right length
  local_address_table[pc_lower_bits] = ((local_address_table[pc_lower_bits] << 1) | outcome)  & (local_bht_entries-1); 

  index = local_address_table[pc_lower_bits];
  //Update local address bht
  switch(local_bht_tournament[index]){
    case WN:
      local_bht_tournament[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      local_bht_tournament[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      local_bht_tournament[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      local_bht_tournament[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in Tournament Global BHT!\n");
  }

  index = ghistory_lower_bits;
  // Update Chooser
  if (local_pred == outcome) {
    if (global_pred != outcome) {
      switch(chooser_bht_tournament[index]){
        case WN:
          chooser_bht_tournament[index] = WT;
          break;
        case SN:
          chooser_bht_tournament[index] = WN;
          break;
        case WT:
          chooser_bht_tournament[index] = ST;
          break;
        case ST:
          chooser_bht_tournament[index] = ST;
          break;
        default:
          printf("Warning: undefined state in Tournament Chooser\n");
      }
    }
  } else {
    if (global_pred == outcome) {
      switch(chooser_bht_tournament[index]){
        case WN:
          chooser_bht_tournament[index] = SN;
          break;
        case SN:
          chooser_bht_tournament[index] = SN;
          break;
        case WT:
          chooser_bht_tournament[index] = WN;
          break;
        case ST:
          chooser_bht_tournament[index] = WT;
          break;
        default:
          printf("Warning: undefined state in Tournament Chooser\n");
      }
    }
  }

  //Update global history register
  thistory = ((thistory << 1) | outcome); 
}

void
cleanup_tournament() {
  free(local_address_table);
  free(local_bht_tournament);
  free(global_bht_tournament);
  free(chooser_bht_tournament);
}

//////////////////////////////////////////////////////////////////////////

//custom skew predictor functions
void init_skew() {
 int bht_entries = 1 << skewHistoryBits;
 int bht_entries_ext = 1 << (skewHistoryBits + 1);
  skew_bht_1 = (uint8_t*)malloc(bht_entries_ext * sizeof(uint8_t));
  skew_bht_2 = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  skew_bht_3 = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    skew_bht_2[i] = WN;
    skew_bht_3[i] = WN;
  }
  for(i = 0; i< bht_entries_ext; i++){
    skew_bht_1[i] = WN;
  }
  shistory = 0;
}



uint8_t 
skew_predict(uint32_t pc) {
  uint32_t bht_entries = 1 << skewHistoryBits;
  uint32_t bht_entries_ext = 1 << (skewHistoryBits + 1);
  uint32_t half_bitmask = 1 << (skewHistoryBits - 6);
  uint32_t pc_lower_bits1 = pc & (bht_entries-1);
  uint32_t pc_lower_bits_ext = pc & (bht_entries_ext - 1);
  uint32_t pc_lower_bits2 = pc & (half_bitmask-1);
  uint32_t shistory_lower_bits1 = shistory & (bht_entries -1);
  uint32_t shistory_lower_bits_ext = shistory & (bht_entries_ext -1);
  uint32_t shistory_lower_bits2 = shistory & (half_bitmask - 1);

  // Indices used are: standard gshare, and two ways to concatenate address and history
  uint32_t index1 = pc_lower_bits_ext ^ shistory_lower_bits_ext;
  uint32_t index_base = pc_lower_bits1 ^ shistory_lower_bits1;
  // uint32_t index2 = (pc_lower_bits2 << 6) | shistory_lower_bits2;
  uint32_t index2 = index_base ^ pc_lower_bits2;
  // uint32_t index3 = (pc_lower_bits2) | (shistory_lower_bits2 << 6);
  uint32_t index3 = index_base ^ (pc_lower_bits2 << 6);


  int choice = 0;
  switch(skew_bht_1[index1]){
    case WN:
      choice -= 1;
      break;
    case SN:
      choice -= 1;
      break;
    case WT:
      choice += 1;
      break;
    case ST:
      choice += 1;
      break;
    default:
      printf("Warning: Undefined state of entry in Skew1 BHT!\n");
      return NOTTAKEN;
  }
  switch(skew_bht_2[index2]){
    case WN:
      choice -= 1;
      break;
    case SN:
      choice -= 1;
      break;
    case WT:
      choice += 1;
      break;
    case ST:
      choice += 1;
      break;
    default:
      printf("Warning: Undefined state of entry in Skew2 BHT!\n");
      return NOTTAKEN;
  }
  switch(skew_bht_3[index3]){
    case WN:
      choice -= 1;
      break;
    case SN:
      choice -= 1;
      break;
    case WT:
      choice += 1;
      break;
    case ST:
      choice += 1;
      break;
    default:
      printf("Warning: Undefined state of entry in Skew3 BHT!\n");
      return NOTTAKEN;
  }

  if (choice > 0) {
    return TAKEN;
  } else {
    return NOTTAKEN;
  }
}

void
train_skew(uint32_t pc, uint8_t outcome) {
  uint32_t bht_entries = 1 << skewHistoryBits;
  uint32_t bht_entries_ext = 1 << (skewHistoryBits + 1);
  uint32_t half_bitmask = 1 << (skewHistoryBits - 6);
  uint32_t pc_lower_bits1 = pc & (bht_entries-1);
  uint32_t pc_lower_bits_ext = pc & (bht_entries_ext - 1);
  uint32_t pc_lower_bits2 = pc & (half_bitmask-1);
  uint32_t shistory_lower_bits1 = shistory & (bht_entries -1);
  uint32_t shistory_lower_bits_ext = shistory & (bht_entries_ext -1);
  uint32_t shistory_lower_bits2 = shistory & (half_bitmask - 1);

  // Indices used are: standard gshare, and two ways to concatenate address and history
  uint32_t index1 = pc_lower_bits_ext ^ shistory_lower_bits_ext;
  uint32_t index_base = pc_lower_bits1 ^ shistory_lower_bits1;
  // uint32_t index2 = (pc_lower_bits2 << 6) | shistory_lower_bits2;
  uint32_t index2 = index_base ^ pc_lower_bits2;
  // uint32_t index3 = (pc_lower_bits2) | (shistory_lower_bits2 << 6);
  uint32_t index3 = index_base ^ (pc_lower_bits2 << 6);

  //Update state of entry in bht based on outcome
  int choice = 0;
  uint8_t pred1;
  uint8_t pred2;
  uint8_t pred3;
  switch(skew_bht_1[index1]){
    case WN:
      choice -= 1;
      pred1 = NOTTAKEN;
      break;
    case SN:
      choice -= 1;
      pred1 = NOTTAKEN;
      break;
    case WT:
      choice += 1;
      pred1 = TAKEN;
      break;
    case ST:
      choice += 1;
      pred1 = TAKEN;
      break;
    default:
      printf("Warning: Undefined state of entry in Skew1 BHT!\n");
      pred1 =  NOTTAKEN;
  }
  switch(skew_bht_2[index2]){
    case WN:
      choice -= 1;
      pred2 = NOTTAKEN;
      break;
    case SN:
      choice -= 1;
      pred2 = NOTTAKEN;
      break;
    case WT:
      choice += 1;
      pred2 = TAKEN;
      break;
    case ST:
      choice += 1;
      pred2 = TAKEN;
      break;
    default:
      printf("Warning: Undefined state of entry in Skew2 BHT!\n");
      pred2 =  NOTTAKEN;
  }
  switch(skew_bht_3[index3]){
    case WN:
      choice -= 1;
      pred3 = NOTTAKEN;
      break;
    case SN:
      choice -= 1;
      pred3 = NOTTAKEN;
      break;
    case WT:
      choice += 1;
      pred3 = TAKEN;
      break;
    case ST:
      choice += 1;
      pred3 = TAKEN;
      break;
    default:
      printf("Warning: Undefined state of entry in Skew3 BHT!\n");
      pred3 = NOTTAKEN;
  }

  uint8_t taken;

  if (choice > 0) {
    taken = TAKEN;
  } else {
    taken = NOTTAKEN;
  }

// if correct, only update the correct ones
  if (taken == outcome) {
    if (pred1 == outcome){
      switch(skew_bht_1[index1]){
        case WN:
          skew_bht_1[index1] = (outcome==TAKEN)?WT:SN;
          break;
        case SN:
          skew_bht_1[index1] = (outcome==TAKEN)?WN:SN;
          break;
        case WT:
          skew_bht_1[index1] = (outcome==TAKEN)?ST:WN;
          break;
        case ST:
          skew_bht_1[index1] = (outcome==TAKEN)?ST:WT;
          break;
        default:
          printf("Warning: Undefined state of entry in skew1 BHT!\n");
      }
    }
    if (pred2 == outcome) {
      switch(skew_bht_2[index2]){
        case WN:
          skew_bht_2[index2] = (outcome==TAKEN)?WT:SN;
          break;
        case SN:
          skew_bht_2[index2] = (outcome==TAKEN)?WN:SN;
          break;
        case WT:
          skew_bht_2[index2] = (outcome==TAKEN)?ST:WN;
          break;
        case ST:
          skew_bht_2[index2] = (outcome==TAKEN)?ST:WT;
          break;
        default:
          printf("Warning: Undefined state of entry in skew2 BHT!\n");
      }
    }
    if (pred3 == outcome) {
      switch(skew_bht_3[index3]){
        case WN:
          skew_bht_3[index3] = (outcome==TAKEN)?WT:SN;
          break;
        case SN:
          skew_bht_3[index3] = (outcome==TAKEN)?WN:SN;
          break;
        case WT:
          skew_bht_3[index3] = (outcome==TAKEN)?ST:WN;
          break;
        case ST:
          skew_bht_3[index3] = (outcome==TAKEN)?ST:WT;
          break;
        default:
          printf("Warning: Undefined state of entry in skew3 BHT!\n");
      }
    }
  }  else {
    switch(skew_bht_1[index1]){
      case WN:
        skew_bht_1[index1] = (outcome==TAKEN)?WT:SN;
        break;
      case SN:
        skew_bht_1[index1] = (outcome==TAKEN)?WN:SN;
        break;
      case WT:
        skew_bht_1[index1] = (outcome==TAKEN)?ST:WN;
        break;
      case ST:
        skew_bht_1[index1] = (outcome==TAKEN)?ST:WT;
        break;
      default:
        printf("Warning: Undefined state of entry in skew1 BHT!\n");
    }
    switch(skew_bht_2[index2]){
      case WN:
        skew_bht_2[index2] = (outcome==TAKEN)?WT:SN;
        break;
      case SN:
        skew_bht_2[index2] = (outcome==TAKEN)?WN:SN;
        break;
      case WT:
        skew_bht_2[index2] = (outcome==TAKEN)?ST:WN;
        break;
      case ST:
        skew_bht_2[index2] = (outcome==TAKEN)?ST:WT;
        break;
      default:
        printf("Warning: Undefined state of entry in skew1 BHT!\n");
    }
    switch(skew_bht_3[index3]){
      case WN:
        skew_bht_3[index3] = (outcome==TAKEN)?WT:SN;
        break;
      case SN:
        skew_bht_3[index3] = (outcome==TAKEN)?WN:SN;
        break;
      case WT:
        skew_bht_3[index3] = (outcome==TAKEN)?ST:WN;
        break;
      case ST:
        skew_bht_3[index3] = (outcome==TAKEN)?ST:WT;
        break;
      default:
        printf("Warning: Undefined state of entry in skew1 BHT!\n");
    }
  }


  //Update history register
  shistory = ((shistory << 1) | outcome); 
}

void
cleanup_skew() {
  free(skew_bht_1);
  free(skew_bht_2);
  free(skew_bht_3);
}


void
init_predictor()
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
      init_tournament();
      break;
    case CUSTOM:
      init_skew();
    default:
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return tournament_predict(pc);
    case CUSTOM:
      return skew_predict(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void
train_predictor(uint32_t pc, uint8_t outcome)
{

  switch (bpType) {
    case STATIC:
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tournament(pc, outcome);
    case CUSTOM:
      return train_skew(pc, outcome);
    default:
      break;
  }
  

}
