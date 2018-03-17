//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"SerialBF.h"
#include"helperFunctions.h"
#include"murmur.h"
#include<cstdlib>
#include<ctime>
#include<cstring>
#include<cstdio>

SerialBF::
SerialBF(const size_t size,
	const size_t k) {

	//initialize bit array
	bitArray = new BYTE[size];
	bitArrLen = (uint64_t)size * sizeof(BYTE) * 8;
	std::memset(bitArray, 0, size);

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

SerialBF::
~SerialBF() {
	delete []bitArray;

#ifdef DISABLE_TWO_PHASE
	delete []seeds;
#endif
}

void SerialBF::
add(const void* key,
	const int len) {

#ifdef DISABLE_TWO_PHASE
	for (size_t i = 0; i < numHashes; ++i) {
		uint64_t out[2];
		MurmurHash3_x64_128(key, len, seeds[i], out);
		uint64_t bitId = out[0] % bitArrLen;
#ifdef DEBUG
		std::printf("Insertion Key=%d, Set Bit=%ld\n",(int)(*((BYTE*)key)), bitId);
#endif

		setBit(bitArray, bitId);
	}
#endif

}

bool SerialBF::
query(const void* key,
	const int len) {

#ifdef DISABLE_TWO_PHASE
	for (size_t i = 0; i < numHashes; ++i) {
		uint64_t out[2];
		MurmurHash3_x64_128(key, len, seeds[i], out);
		uint64_t bitId = out[0] % bitArrLen;
#ifdef DEBUG
		std::printf("Query Key=%d, Bitid=%ld\n",(int)(*((BYTE*)key)),bitId);
#endif
		if (!testBit(bitArray, bitId)) {
			return false;
		}
	}
	return true;
#endif
}

void SerialBF::
add_batch(const void* keys,
	const int batchLen,
	const int keyLen) {
	const BYTE* keyArr = (const BYTE*)keys;
	for (int i = 0; i < batchLen; ++i) {
		add(&keyArr[i * keyLen], keyLen);
	}
}

void SerialBF::
query_batch(const void* keys,
	BYTE* results,
	const int batchLen,
	const int keyLen) {
	const BYTE* keyArr = (const BYTE*)keys;
	for (int i = 0; i < batchLen; ++i) {
		if (query(&keyArr[i*keyLen], keyLen)) {
			setBit(results, i);
		}
	}
}
