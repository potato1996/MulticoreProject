//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"testCase.h"
#include"ParallelBFWNOrder.h"
#include"SerialBF.h"
#include"Common.h"
#include<cstdlib>
void testUTAPBF(const int threadNum,
	const int testNum,
	const size_t BFsize = BF_DEFAULT_BYTES,
	const size_t BFhashNum = BF_DEFAULT_K) {
	CommonFilter* cf = new ParallelBFWNOrder(BFsize, BFhashNum, threadNum);
	CommonFilter* cfBaseline = new SerialBF(BFsize,BFhashNum);
	testCase* testCases = new testCase();
	testCases->generate(ELE_DEFAULT_BYTES, testNum, testNum, 0.8);
	testCases->runTest(cfBaseline, true);
	testCases->runTest(cf, true);
	delete testCases;
	delete cfBaseline;
	delete cf;
}

int main(int argc, char** argv) {
	
	const int threadNum = atoi(argv[1]);
	const int BFsize = atoi(argv[2]);
	const int BFhashNum = atoi(argv[3]);
	const int testNum = atoi(argv[4]);
	testUTAPBF(threadNum, testNum, BFsize, BFhashNum);
	return 0;
}
