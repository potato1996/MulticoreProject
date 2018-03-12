//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

//To make a "consist meaning" of the false rate, this calss only 
//test a batch of add followed by a batch of query.
//This class DO focuns on testing efficiency and false rate using
//a large test dataset.
//SHOULD USE OTHER UNIT TESTS TO VALIDATE CONSISTENCY OF THE "MIXED"
//ADD/QUERY CASES.
#include"Common.h"
#include"CommonFilter.h"
#include<vector>
#pragma once
class testCase {
public:
	testCase();

	testCase(const testCase&) = delete;
		
	void generate(const size_t keyLen, //in bytes
		const int numAdd, //number of elements to put into filter(n)
		const int numQuery, //number of querys
		const double hitRate); //query hit rate

	void runTest(CommonFilter* f,
		bool validateResult = false);

	~testCase();

private:
	size_t keyLen;//in bytes
	int numAdd;//number of elements to put into filter(n). Should be a multiply of 8
	int numQuery;//number of querys. Should be a multiply of 8.
	BYTE* addKeys; //(generated) keys for adding into filter
	BYTE* queryKeys; //(generated) keys for queries
	BYTE* queryAns;  //standard ans for queried keys.



};