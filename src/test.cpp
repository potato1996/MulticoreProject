//----------------------------------------------------
//Author: Dayou Du(2018) 
//dayoudu@nyu.edu

#include"SerialBF.h"
void testSerialBF(const int testNum,
	const size_t size = BF_DEFAULT_BYTES,
	const size_t k = BF_DEFAULT_K) {
	SerialBF bf(size, k);
	int64_t key[2];
	for (int i = 0; i < testNum; ++i) {
		key[1] = i;
		bf.add(key, 16);
	}
}

int main() {
	testSerialBF(20,3,2);
}