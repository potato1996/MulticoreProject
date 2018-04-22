//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"testCase.h"
#include"ParallelBFWNOrder.h"
#include"ParallelQFWNOrder.h"
#include"SerialBF.h"
#include"SerialQF.h"
#include"Common.h"
#include<cstdio>
#include<cstdlib>
#include<string.h>

void testUTAPBF(const int threadNum,
	const int testNum,
	const size_t BFsize = BF_DEFAULT_BYTES,
	const size_t BFhashNum = BF_DEFAULT_K) {

	printf("---------- Start Testing Bloom Filter ----------\n");

	auto cf = new ParallelBFWNOrder(BFsize, BFhashNum, threadNum);
	auto cfExtraMem = new ParallelBFWNOrder(BFsize, BFhashNum, threadNum, true);
	auto cfBaseline = new SerialBF(BFsize,BFhashNum);

#ifdef DISABLE_TWO_PHASE
	cf->forceSetSeeds(cfBaseline->getSeeds());
	cfExtraMem->forceSetSeeds(cfBaseline->getSeeds());
#else
	cf->forceSetSeed(cfBaseline->getSeed());
	cfExtraMem->forceSetSeed(cfBaseline->getSeed());
#endif

	testCase* testCases = new testCase();
	testCases->generate(ELE_DEFAULT_BYTES, testNum, testNum, 0.8);
	testCases->runTest(cfBaseline, true);
	testCases->runTest(cf, true);
	testCases->runTest(cfExtraMem, true);
	delete testCases;
	delete cfBaseline;
	delete cf;
}

void testUTAPQF(const int threadNum,
	const int testNum,
	const size_t q = QF_DEFAULT_QBITS,
	const size_t r = QF_DEFAULT_RBITS) {

	printf("---------- Start Testing Quotient Filter ----------\n");

	auto cf = new ParallelQFWNOrder(q, r, threadNum);
	auto cfBaseline = new SerialQF(q, r);

	cf->forceSetSeed(cfBaseline->getSeed());
	
	testCase* testCases = new testCase();
	testCases->generate(ELE_DEFAULT_BYTES, testNum, testNum, 0.8);
	testCases->runTest(cfBaseline, true);
	testCases->runTest(cf, true);
	delete testCases;
	delete cfBaseline;
	delete cf;
}

int main(int argc, char** argv) {
	int threadNum = PBF_DEFAULT_TN;
	int testNum = DEFAULT_NUM_TESTS;

	//BF
	int BFsize = BF_DEFAULT_BYTES;
	int BFhashNum = BF_DEFAULT_K;

	//QF
	int qbits = QF_DEFAULT_QBITS;
	int rbits = QF_DEFAULT_RBITS;

	if (argc < 3) { 
        printf("Usage is -t <numThreads> -tc <numTestCases> -bs <BFsize> -k <BFNumHash> -q <QFQuoSize> -r <QFRemSize>\n"); 
        return 0;
	}
	
	for (int i = 1; i < argc; i+=2) {   
    	int val = atoi(argv[i+1]);
    	if (strcmp(argv[i], "-t") == 0) {
            threadNum = val;
        } else if (strcmp(argv[i], "-tc") == 0) {
            testNum = val;
        } else if (strcmp(argv[i], "-bs") == 0) {
            BFsize = val;
        } else if (strcmp(argv[i], "-k") == 0) {
            BFhashNum = val;
        } else if (strcmp(argv[i], "-q") == 0) {
            qbits = val;
        } else if (strcmp(argv[i], "-r") == 0) {
            rbits = val;
        }   
    }

	testUTAPBF(threadNum, testNum, BFsize, BFhashNum);
	testUTAPQF(threadNum, testNum, qbits, rbits);

	return 0;
}
