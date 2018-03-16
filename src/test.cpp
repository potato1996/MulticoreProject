//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"testCase.h"
#include"ParallelBFWNOrder.h"
#include<cstdlib>
void testUTAPBF(const int threadNum,
	const int testNum,
	const size_t size = BF_DEFAULT_BYTES,
	const size_t k = BF_DEFAULT_K) {
	CommonFilter* cf = new ParallelBFWNOrder(size, k, threadNum);
	testCase* testCases = new testCase();
	testCases->generate(size, testNum, testNum, 0.8);
	testCases->runTest(cf, true);
	delete testCases;
	delete cf;
}

int main(int argc, char** argv) {
	
	const int threadNum = atoi(argv[1]);
	const int testNum = atoi(argv[2]);
	const int keyLen = atoi(argv[3]);
	const int k = atoi(argv[4]);
	testUTAPBF(threadNum, testNum, keyLen, k);
	return 0;
}