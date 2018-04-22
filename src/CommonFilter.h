//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

/** CommonFilter: Common Interface of AMQs
 ** 
 ** add: insert a key with length keyLen bytes into the filter
 ** 
 ** query: check whether a key with length keyLen is in the filter
 **
 ** add_batch: insert batchLen of keys, each key have length keyLen.
 **
 ** query_batch: check batchLen of keys are in the filter. The result
 **              for each key is stored as a bit array "results"
 **/

#pragma once
#include"Common.h"
class CommonFilter {
public:


	virtual void add(const void* key,
		const int keyLen) = 0; //key length in bytes

	virtual bool query(const void* key,
		const int keyLen) = 0; //key length in bytes

	virtual void add_batch(const void* keys,
		const int batchLen, //number of elements to put into filter(n)
		const int keyLen) = 0;

	virtual void query_batch(const void* keys,
		uint8_t* results,
		const int batchLen, //number of querys
		const int keyLen) = 0;

	virtual ~CommonFilter() {}
};
