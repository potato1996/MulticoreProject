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
#include<cstdio>
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
	const int _tn,
	bool enableExtraMemory){

	//initialize bit array
	bitArray = new BYTE[size];
	bitArrLen = (uint64_t)size * sizeof(BYTE) * 8;
	std::memset(bitArray, 0, size);
	threadNum = _tn;
	useExtraMemory = enableExtraMemory;

	numHashes = k;
#ifdef DISABLE_TWO_PHASE
	//initialize seeds
	seeds = new uint32_t[k];
	std::srand(std::time(nullptr));
	for (size_t i = 0; i < k; ++i) {
		seeds[i] = (uint32_t)std::rand();
	}
#else
	std::srand(std::time(nullptr));
	seed = (uint32_t)std::rand();
#endif //DISABLE_TWO_PHASE

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
#ifdef DEBUG
		std::printf("Query Key=%d, Bitid=%ld\n",(int)(*((BYTE*)key)),bitId);
#endif //DEBUG
		if (!testBit(bitArray, bitId)) {
			return false;
		}
	}
	return true;
#else  //DISABLE_TWO_PHASE
	uint64_t out[2];
	MurmurHash3_x64_128(key, len, seed, out);
	for(size_t i = 0; i < numHashes; ++i){
		
		uint64_t combineHash = out[0] + i * out[1];
		uint64_t bitId = combineHash % bitArrLen;

#ifdef DEBUG
		std::printf("Query Key=%d, Bitid=%ld\n",(int)(*((BYTE*)key)),bitId);
#endif  //DEBUG
		if (!testBit(bitArray, bitId)) {
			return false;
		}
	}
	return true;
#endif //DISABLE_TWO_PHASE

}

void ParallelBFWNOrder::
add_batch(const void * keys, 
	const int batchLen, 
	const int keyLen){

	const BYTE* keyArr = (const BYTE*)keys;

	//TODO: Get a more "explainable" chunk size based on calculation
	//int chunkSize = std::min(1000, (batchLen - 1) / threadNum + 1);

	omp_set_num_threads(threadNum);

	if (useExtraMemory) {
		//add to own bit array, and then gather up
		BYTE* privateBitArr = new BYTE[bitArrLen];
#pragma omp parallel
		{
			BYTE* ownBitArr = privateBitArr + bitArrLen * omp_get_thread_num();
#pragma omp for schedule(runtime)
			for (int i = 0; i < batchLen; ++i) {

#ifdef DISABLE_TWO_PHASE

				for (size_t k = 0; k < numHashes; ++k) {

					//calculate hash(i,k)
					uint64_t out[2];
					MurmurHash3_x64_128(&keyArr[i * keyLen], keyLen, seeds[k], out);
					uint64_t posId = out[0] % bitArrLen;

#ifdef DEBUG
					std::printf("Insertion Key=%d, Set Bit=%ld\n", (int)keyArr[i*keyLen], posId);
#endif //DEBUG

					//set bit
					uint64_t byteId = posId / 8;
					uint64_t bitId = posId % 8;

					//write to own bit array, do not need atomic
					ownBitArr[byteId] |= mask[bitId];
				}

#else 

				uint64_t out[2];
				MurmurHash3_x64_128(&keyArr[i * keyLen], keyLen, seed, out);

				for (size_t k = 0; k < numHashes; ++k) {
					uint64_t combineHash = out[0] + k * out[1];
					uint64_t posId = combineHash % bitArrLen;

#ifdef DEBUG
					std::printf("Insertion Key=%d, Set Bit=%ld\n", (int)keyArr[i*keyLen], posId);
#endif //DEBUG

					//set bit
					uint64_t byteId = posId / 8;
					uint64_t bitId = posId % 8;
					
					//write to own bit array, do not need atomic
					ownBitArr[byteId] |= mask[bitId];
				}

#endif //DISABLE_TWO_PHASE
			}
			uint64_t* llprivBitArr = (uint64_t*)privateBitArr;
			uint64_t* llbitArr = (uint64_t*)bitArray;
#pragma omp for schedule(runtime)
			for (int i = 0; i < bitArrLen / 8; ++i) {
				uint64_t res = 0;
				for (int j = 0; j < threadNum; ++j) {
					res |= llprivBitArr[j * bitArrLen / 8 + i];
				}
				llbitArr[i] |= res;
			}
		}
		delete[]privateBitArr;
	}
	else {
#pragma omp parallel for schedule(runtime)
		for (int i = 0; i < batchLen; ++i) {

#ifdef DISABLE_TWO_PHASE

			for (size_t k = 0; k < numHashes; ++k) {

				//calculate hash(i,k)
				uint64_t out[2];
				MurmurHash3_x64_128(&keyArr[i * keyLen], keyLen, seeds[k], out);
				uint64_t posId = out[0] % bitArrLen;

#ifdef DEBUG
				std::printf("Insertion Key=%d, Set Bit=%ld\n", (int)keyArr[i*keyLen], posId);
#endif //DEBUG

				//set bit
				uint64_t byteId = posId / 8;
				uint64_t bitId = posId % 8;
#pragma omp atomic
				bitArray[byteId] |= mask[bitId];
			}

#else 

			uint64_t out[2];
			MurmurHash3_x64_128(&keyArr[i * keyLen], keyLen, seed, out);

			for (size_t k = 0; k < numHashes; ++k) {
				uint64_t combineHash = out[0] + k * out[1];
				uint64_t posId = combineHash % bitArrLen;

#ifdef DEBUG
				std::printf("Insertion Key=%d, Set Bit=%ld\n", (int)keyArr[i*keyLen], posId);
#endif //DEBUG

				//set bit
				uint64_t byteId = posId / 8;
				uint64_t bitId = posId % 8;
#pragma omp atomic
				bitArray[byteId] |= mask[bitId];
			}

#endif //DISABLE_TWO_PHASE

		}
	}

}

void ParallelBFWNOrder::
query_batch(const void * keys, 
	BYTE * results, 
	const int batchLen, 
	const int keyLen){

	const BYTE* keyArr = (const BYTE*)keys;

	//TODO: Get a more "explainable" chunk size based on calculation
	//const int chunkSize = std::max(1, std::min(1000, (batchLen - 1) / threadNum + 1));
	int chunkSize = (batchLen - 1)/threadNum + 1;
	
	//ensure that chunkSize is a multiply of 8
	chunkSize = ((chunkSize - 1) / 8 + 1) * 8;
	
	omp_set_num_threads(threadNum);

#pragma omp parallel for schedule(static, chunkSize)
	for (int i = 0; i < batchLen; ++i) {
		if (query(&keyArr[i*keyLen], keyLen)) {
			//set result
			uint64_t byteId = i / 8;
			uint64_t bitId = i % 8;
			#ifdef DEBUG
				std::printf("Query Key=%d, Claim In Set\n",(int)keyArr[i*keyLen]);
			#endif

			//here we do NOT need an atomic operation.
			//Because we ensured that chunksize is a mutiply of 8
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
