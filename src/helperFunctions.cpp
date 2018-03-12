#include"helperFunctions.h"
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
void setBit(BYTE* arr, uint64_t i) {
	uint64_t byteId = i / 8;
	uint64_t bitId = i % 8;
	arr[byteId] |= mask[bitId];
}
bool testBit(BYTE* arr, uint64_t i) {
	uint64_t byteId = i / 8;
	uint64_t bitId = i % 8;
	return arr[byteId] & mask[bitId];
}