//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"ParallelBFWNOrder.h"
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

	//NOTE: From profiling result we find out that no "schedule" clause
	//      is better than everything else..

	omp_set_num_threads(threadNum);

	if (useExtraMemory) {
		//add to own bit array, and then gather up
		BYTE* privateBitArr = new BYTE[bitArrLen/8 * threadNum];
#pragma omp parallel
		{
			BYTE* ownBitArr = privateBitArr + bitArrLen/8 * omp_get_thread_num();
			memset(ownBitArr, 0 , bitArrLen/8);
#pragma omp for
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
			//merge using 64bit - a great speed up of merging
			uint64_t* llprivBitArr = (uint64_t*)privateBitArr;
			uint64_t* llbitArr = (uint64_t*)bitArray;
#pragma omp for
			for (uint64_t i = 0; i < bitArrLen / 64; ++i) {
				uint64_t res = 0;
				for (int j = 0; j < threadNum; ++j) {
					res |= llprivBitArr[j * bitArrLen / 64 + i];
				}
				llbitArr[i] |= res;
			}
		}
		delete[]privateBitArr;
	}
	else {
#pragma omp parallel for
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

    //NOTE: Regarding the input batchLen
	//1.    Since we use a bit array to store the result,
	//      We have to correctly deal with the racing conditions
	//      More precisly, we garantee that each thread get a 
	//      chunksize with a multiply of 8.
	//
	//2.    The results in reality should be a boolean array,
	//      here we just would like to save time on testing 
	//      - generating the test case inputs.
	//
	//3.    To summarize, this racing condition will NOT actually
	//      show up in reality, and has nothing to do with the algorithm.
	//    **It is just a side effect of how to store the results**

    //ASSERT((batchLen % (8 * threadNum)) == 0)

	//NOTE: Here we really do not need a chunksize.
	//1.    From profiling and tests we know that with no "schedule" clause
	//      is better than everything else.
	//2.    We have already garanteed that the chunksize for each 
	//      thread is a multiply of 8.
	
	omp_set_num_threads(threadNum);

#pragma omp parallel for 
	for (int i = 0; i < batchLen; ++i) {
		if (query(&keyArr[i*keyLen], keyLen)) {
			//set result
			uint64_t byteId = i / 8;
			uint64_t bitId = i % 8;
			#ifdef DEBUG
				std::printf("Query Key=%d, Claim In Set\n",(int)keyArr[i*keyLen]);
			#endif
			
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
