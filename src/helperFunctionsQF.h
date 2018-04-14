//----------------------------------------------------
//Author: Yik Wai Ng(2018) 
//ywn202@nyu.edu

#pragma once
#include"Common.h"
uint64_t increment(uint64_t index, uint64_t qmask);
uint64_t decrement(uint64_t index, uint64_t qmask);
uint64_t hash(const void* key, const int len, uint32_t seed);
uint64_t hash_to_quotient(uint64_t hash, uint8_t rbits, uint64_t qmask);
uint64_t hash_to_remainder(uint64_t hash, uint64_t rmask);
int is_occupied(uint64_t elt);
uint64_t set_occupied(uint64_t elt);
uint64_t clr_occupied(uint64_t elt);
int is_continuation(uint64_t elt);
uint64_t set_continuation(uint64_t elt);
uint64_t clr_continuation(uint64_t elt);
int is_shifted(uint64_t elt);
uint64_t set_shifted(uint64_t elt);
uint64_t clr_shifted(uint64_t elt);
uint64_t get_remainder(uint64_t elt);
bool is_empty_element(uint64_t elt);
bool is_cluster_start(uint64_t elt);
bool is_run_start(uint64_t elt);
uint64_t table_size(uint32_t qbits, uint32_t rbits);