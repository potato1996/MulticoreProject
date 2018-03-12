//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include "ParallelBFWNOrder.h"
#include"helperFunctions.h"
#include"murmur.h"
#include<cstdlib>
#include<ctime>
#include<cstring>
#include<omp.h>
#include<algorithm>

namespace {
	const BYTE mask[8] = {
		1,
		1 << 1,
		1 << 2,
		1 << 3,
		1 << 4,
		1 << 5,
		1 << 6,
		1 << 7
	};
}

ParallelBFWNOrder::
ParallelBFWNOrder(const size_t size, 
	const size_t k,
	const int _tn){

	//initialize bit array
	bitArray = new BYTE[size];
	bitArrLen = (uint64_t)size * sizeof(BYTE) * 8;
	std::memset(bitArray, 0, size);
	threadNum = _tn;

#ifdef DISABLE_TWO_PHASE
	//initialize seeds
	numHashes = k;
	seeds = new uint32_t[k];
	std::srand(std::time(nullptr));
	for (size_t i = 0; i < k; ++i) {
		seeds[i] = (uint32_t)std::rand();
	}
#endif
}

void ParallelBFWNOrder::
add(const void * key, 
	const int len){

	//call batch on 1 element..
	add_batch(key, 1, len);
}

bool ParallelBFWNOrder::
query(const void * key, 
	const int len){

#ifdef DISABLE_TWO_PHASE
	for (size_t i = 0; i < numHashes; ++i) {
		uint64_t out[2];
		MurmurHash3_x64_128(key, len, seeds[i], out);
		uint64_t bitId = out[0] % bitArrLen;
		if (!testBit(bitArray, bitId)) {
			return false;
		}
	}
	return true;
#endif

}

void ParallelBFWNOrder::
add_batch(const void * keys, 
	const int batchLen, 
	const int keyLen){

	const BYTE* keyArr = (const BYTE*)keys;

	//TODO: Get a more "explainable" chunk size based on calculation
	const int chunkSize = std::max(8, std::min(1000, (batchLen - 1) / threadNum + 1));

	omp_set_num_threads(threadNum);

#pragma omp parallel for schedule(static,chunkSize)
	for (int i = 0; i < batchLen; ++i) {

#ifdef DISABLE_TWO_PHASE
		for (size_t k = 0; k < numHashes; ++k) {

			//calculate hash(i,k)
			uint64_t out[2];
			MurmurHash3_x64_128(&keyArr[i * keyLen], keyLen, seeds[k], out);
			uint64_t posId = out[0] % bitArrLen;

			//set bit
			uint64_t byteId = posId / 8;
			uint64_t bitId = posId % 8;
			
			//here we do NOT need an atomic operation. Because each chunk has at least 8 elements.
			bitArray[byteId] |= mask[bitId];

		}
#endif

	}

}

void ParallelBFWNOrder::
query_batch(const void * keys, 
	BYTE * results, 
	const int batchLen, 
	const int keyLen){

	const BYTE* keyArr = (const BYTE*)keys;

	//TODO: Get a more "explainable" chunk size based on calculation
	const int chunkSize = std::max(1, std::min(1000, (batchLen - 1) / threadNum + 1));

	omp_set_num_threads(threadNum);

#pragma omp parallel for schedule(static,chunkSize)
	for (int i = 0; i < batchLen; ++i) {
		if (query(&keyArr[i*keyLen], keyLen)) {
			//set result
			uint64_t byteId = i / 8;
			uint64_t bitId = i % 8;
			#pragma omp atomic
				results[byteId] |= mask[bitId];
		}
	}
}

ParallelBFWNOrder::
~ParallelBFWNOrder(){
	delete[]bitArray;

#ifdef DISABLE_TWO_PHASE
	delete[]seeds;
#endif
}