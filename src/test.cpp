//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"testCase.h"
#include"ParallelBFWNOrder.h"
#include"SerialBF.h"
#include"SerialQF.h"
#include"Common.h"
#include<cstdio>
#include<cstdlib>

void testUTAPBF(const int threadNum,
	const int testNum,
	const size_t BFsize = BF_DEFAULT_BYTES,
	const size_t BFhashNum = BF_DEFAULT_K) {
	auto cf = new ParallelBFWNOrder(BFsize, BFhashNum, threadNum);
	auto cfBaseline = new SerialBF(BFsize,BFhashNum);

#ifdef DISABLE_TWO_PHASE
	cf->forceSetSeeds(cfBaseline->getSeeds());
#else
	cf->forceSetSeed(cfBaseline->getSeed());
#endif

	testCase* testCases = new testCase();
	testCases->generate(ELE_DEFAULT_BYTES, testNum, testNum, 0.8);
	testCases->runTest(cfBaseline, true);
	testCases->runTest(cf, true);
	delete testCases;
	delete cfBaseline;
	delete cf;
}

void testUTAPQF(const int threadNum,
	const int testNum,
	const size_t q = QF_DEFAULT_QBITS,
	const size_t r = QF_DEFAULT_RBITS) {
	//auto cf = new ParallelQFWNOrder( );
	auto cfBaseline = new SerialQF(q, r);
	
	testCase* testCases = new testCase();
	testCases->generate(ELE_DEFAULT_BYTES, testNum, testNum, 0.8);
	testCases->runTest(cfBaseline, true);
	//testCases->runTest(cf, true);
	delete testCases;
	delete cfBaseline;
	//delete cf;
}

int main(int argc, char** argv) {
	const int threadNum = atoi(argv[1]);
	const int BFsize = atoi(argv[2]);
	const int BFhashNum = atoi(argv[3]);
	const int testNum = atoi(argv[4]);
	testUTAPBF(threadNum, testNum, BFsize, BFhashNum);
	testUTAPQF(threadNum, testNum);
	return 0;
}
