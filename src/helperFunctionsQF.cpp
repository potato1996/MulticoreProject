//----------------------------------------------------
//Author: Yik Wai Ng(2018) 
//ywn202@nyu.edu

#include"helperFunctionsQF.h"
#include"murmur.h"
uint64_t increment(uint64_t index, uint64_t qmask) {
	return (index + 1) & qmask;
}
uint64_t decrement(uint64_t index, uint64_t qmask) {
	return (index - 1) & qmask;
}
uint64_t hash(const void* key, const int len, uint32_t seed) {
	uint64_t out[2];
	MurmurHash3_x64_128(key, len, seed, out);
	uint64_t combineHash = out[0] + out[1];
	return combineHash;
}
uint64_t hash_to_quotient(uint64_t hash, uint8_t rbits, uint64_t qmask) {
	return (hash >> rbits) & qmask;
}
uint64_t hash_to_remainder(uint64_t hash, uint64_t rmask) {
	return hash & rmask;
}
int is_occupied(uint64_t element) {
	return element & 1;
}
uint64_t set_occupied(uint64_t element) {
	return element | 1;
}
uint64_t clr_occupied(uint64_t element) {
	return element & ~1;
}
int is_continuation(uint64_t element) {
	return element & 2;
}
uint64_t set_continuation(uint64_t element) {
	return element | 2;
}
uint64_t clr_continuation(uint64_t element) {
	return element & ~2;
}
int is_shifted(uint64_t element) {
	return element & 4;
}
uint64_t set_shifted(uint64_t element) {
	return element | 4;
}
uint64_t clr_shifted(uint64_t element) {
	return element & ~4;
}
uint64_t get_remainder(uint64_t element) {
	return element >> 3;
}
bool is_empty_element(uint64_t element) {
	return (element & 7) == 0;
}
bool is_cluster_start(uint64_t element) {
	return is_occupied(element) && !is_continuation(element) && !is_shifted(element);
}
bool is_run_start(uint64_t element) {
	return !is_continuation(element) && (is_occupied(element) || is_shifted(element));
}
size_t table_size(uint32_t qbits, uint32_t rbits) {
	size_t bits = (1 << qbits) * (rbits + 3);
	size_t bytes = bits / 8;
	return (bits % 8) ? (bytes + 1) : bytes;
}