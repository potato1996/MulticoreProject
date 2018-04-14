//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#pragma once
#include<cstdint>
#include<cstdlib>
typedef uint8_t BYTE;

#define DEFAULT_NUM_TESTS 10000000

//default params - BF
#define BF_DEFAULT_BYTES 0x8000000         //128MB
#define BF_DEFAULT_K 8
#define ELE_DEFAULT_BYTES 16
#define PBF_DEFAULT_TN 8

//default params - QF
#define QF_DEFAULT_QBITS 26              //128MB  = (2^q) * (r+3) bits
#define QF_DEFAULT_RBITS 13
#define PQF_DEFAULT_TN 8

//switchs
//#define DEBUG
//#define DISABLE_TWO_PHASE
