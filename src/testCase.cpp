//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"testCase.h"
#include"helperFunctions.h"
#include<ctime>
#include<cstdlib>
#include<cstdio>
#include<cstring>
#include<numeric>
#include<chrono>
#include<climits>
namespace {
	inline BYTE randByte() {
		return (BYTE)(std::rand() % 0xff);
	}
	// has possiblity of p to return true, (1 - p) to return false
	inline bool dice(const double p) {
		return  (1.0 * std::rand() / INT_MAX) <= p;
	}
	inline int randRange(const int k) { //exclusive
		return std::rand() % k;
	}
	inline BYTE bitCount(BYTE n)
	{
		n = (n & 0x55) + ((n >> 1) & 0x55);
		n = (n & 0x33) + ((n >> 2) & 0x33);
		n = (n & 0x0f) + ((n >> 4) & 0x0f);
		return n;
	}
}//anonymous namespace

testCase::
testCase() {
	keyLen = 0;
	numAdd = 0;
	numQuery = 0;
	addKeys = nullptr;
	queryKeys = nullptr;
	queryAns = nullptr;
	std::srand((unsigned int)std::time(nullptr));
}

testCase::
~testCase() {
	if (addKeys) {
		delete[]addKeys;
	}
	if (queryKeys) {
		delete[]queryKeys;
	}
	if (queryAns) {
		delete[]queryAns;
	}
}

void testCase::
generate(const size_t keyLen, //in bytes
	const int numAdd, //number of elements to put into filter(n)
	const int numQuery, //number of querys
	const double hitRate) { //query hit rate

#ifdef DEBUG
	printf("Start Generating Test Cases\n");
	printf("Key Length = %zd Byte(s), # of insertions = %d, # of queries = %d, query hit rate = %lf\n\n",
		keyLen, numAdd, numQuery, hitRate);
#endif // DEBUG

	//initialize class members
	addKeys = new BYTE[numAdd * keyLen];
	queryKeys = new BYTE[numQuery * keyLen];
	queryAns = new BYTE[numQuery / 8];
	memset(queryAns, 0, numQuery / 8);
	this->keyLen = keyLen;
	this->numAdd = numAdd;
	this->numQuery = numQuery;

	//generate add keys
	for (size_t i = 0; i < numAdd*keyLen; ++i) {
		addKeys[i] = randByte();
	}

	//generate queryKeys
	for (int i = 0; i < numQuery; ++i) {
		if (dice(hitRate)) {
			//Generate a hit query
			int keyId = randRange(numAdd);
			for (int j = 0; j < keyLen; ++j) {
				queryKeys[i * keyLen + j] = addKeys[keyId * keyLen + j];
			}
			//mark answer
			setBit(queryAns, i);
		}
		else {
			//Generate a miss query
			//NOTE: Here we just randomly generate a new key - have small posibillity hit
			for (int j = 0; j < keyLen; ++j) {
				queryKeys[i * keyLen + j] = randByte();
			}
		}
	}

#ifdef DEBUG
	printf("Done Generating Test Cases\n\n");
#endif // DEBUG
}

void testCase::
runTest(CommonFilter* f,
	bool validateResult) {

#ifdef DEBUG
	printf("Start Inserting Keys\n");
	printf("Key Length = %zd Byte(s), # of insertions = %d\n\n",
		keyLen, numAdd);
#endif // DEBUG

	//run insertions
	auto startTime = std::chrono::high_resolution_clock::now();
	f->add_batch(addKeys, numAdd, keyLen);
	auto endTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> diff = endTime - startTime;
	printf("Total Insertion Time = %lf\n\n", diff.count());

#ifdef DEBUG
	printf("Start Querying Keys \n");
	printf("Key Length = %zd Byte(s), # of queried = %d\n\n",
		keyLen, numQuery);
#endif // DEBUG

	BYTE* runResult = new BYTE[numQuery / 8];

	//run queries
	startTime = std::chrono::high_resolution_clock::now();
	f->query_batch(queryKeys, runResult, numQuery, keyLen);
	endTime = std::chrono::high_resolution_clock::now();

	diff = endTime - startTime;
	printf("Total Query Time = %lf\n\n", diff.count());

#ifdef DEBUG
	printf("Start Validating Results/Calculating False Positive Rate\n\n");
#endif // DEBUG

	if (validateResult) {
		int count = 0;
		for (int i = 0; i < numQuery/8; ++i) {
			//get diff and count diff
			count += bitCount(runResult[i] ^ queryAns[i]);
		}
		printf("False Positive Rate = %lf\n\n", 1.0 * count / numQuery);
	}

	delete[]runResult;

}
