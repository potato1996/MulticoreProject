//----------------------------------------------------
//Author: Yik Wai Ng(2018) 
//ywn202@nyu.edu

#pragma once
#include"Common.h"
#include"CommonFilter.h"
#include<omp.h>
#include<cstdint>
class ParallelQFWNOrder:public CommonFilter {
private:
	uint64_t get_element(uint64_t index);

	void set_element(uint64_t index, uint64_t element);

	void insert_into(uint64_t s, uint64_t elt);

	uint64_t find_run_index(uint64_t fq);

public:
	ParallelQFWNOrder() = delete;

	ParallelQFWNOrder(const size_t q = QF_DEFAULT_QBITS,
		const size_t r = QF_DEFAULT_RBITS, 
		const int _tn = PQF_DEFAULT_TN);

	ParallelQFWNOrder(const ParallelQFWNOrder&) = delete;

	void add(const void* key, 
		const int len = ELE_DEFAULT_BYTES);

	bool query(const void* key, 
		const int len = ELE_DEFAULT_BYTES);

	void add_batch(const void* keys,
		const int batchLen,
		const int keyLen = ELE_DEFAULT_BYTES);

	void query_batch(const void* keys,
		BYTE* results,
		const int batchLen,
		const int keyLen = ELE_DEFAULT_BYTES);

	~ParallelQFWNOrder();

	uint32_t getSeed() {return seed;}
	
	void forceSetSeed(uint32_t _s){
		seed = _s;
	}

private:
	uint8_t qbits;
	uint8_t rbits;
	uint8_t elem_bits;
	uint64_t qmask;
	uint64_t rmask;
	uint64_t elem_mask;

	uint64_t *table; //quotient filter storage table

	omp_nest_lock_t *lock; //lock the table block by block
	uint64_t lock_size;
	uint32_t lock_block_length;

	uint32_t seed; //hash function seed

};