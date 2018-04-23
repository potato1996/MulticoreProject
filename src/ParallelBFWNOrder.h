//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

/** Parallel version of Bloom Filter 
 ** Inherited from CommonFilter, provide the four methods
 **/

/** The add_back and query_batch is specially parallized 
 ** for **batch-oriented jobs**
 ** i.e. NO order is guranteed in a batch
 **/

#pragma once
#include"CommonFilter.h"
class ParallelBFWNOrder :public CommonFilter {
public:
	ParallelBFWNOrder() = delete;

	ParallelBFWNOrder(const size_t size = BF_DEFAULT_BYTES, //BF bit array length, in bytes
		const size_t k = BF_DEFAULT_K, // number of hash functions
		const int _tn = PBF_DEFAULT_TN,// number of threads
		bool enableExtraMemory = false // if set, use extra memory when add_batch
		                               // will not cost extra memory after add finish.
		);

	ParallelBFWNOrder(const ParallelBFWNOrder&) = delete;

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

	~ParallelBFWNOrder();

#ifdef DISABLE_TWO_PHASE
	uint32_t* getSeeds() { return seeds; }
	void forceSetSeeds(uint32_t* _s) {
		for (size_t i = 0; i < numHashes; ++i)seeds[i] = _s[i];
	}
#else
	uint32_t getSeed() {return seed;}
	void forceSetSeed(uint32_t _s){
		seed = _s;
	}	
#endif

private:
	BYTE* bitArray;
	uint64_t bitArrLen;
	int threadNum;
	bool useExtraMemory;

	size_t numHashes;
#ifdef DISABLE_TWO_PHASE
	uint32_t* seeds;
#else
	uint32_t seed;
#endif

};
