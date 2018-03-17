//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#pragma once
#include"Common.h"
#include"CommonFilter.h"
#include<cstdint>
class SerialBF:public CommonFilter {
public:
	SerialBF() = delete;

	SerialBF(const size_t size = BF_DEFAULT_BYTES,
		const size_t k = BF_DEFAULT_K);

	SerialBF(const SerialBF&) = delete;

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

#ifdef DISABLE_TWO_PHASE
	uint32_t* getSeeds() { return seeds; }
	void forceSetSeeds(uint32_t* _s) {
		for (int i = 0; i < numHashes; ++i)seeds[i] = _s[i];
	}
#endif

	~SerialBF();

private:
	BYTE* bitArray;
	uint64_t bitArrLen;

#ifdef DISABLE_TWO_PHASE
	size_t numHashes;
	uint32_t* seeds;
#endif

};