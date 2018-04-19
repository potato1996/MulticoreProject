//----------------------------------------------------
//Author: Yik Wai Ng(2018) 
//ywn202@nyu.edu

/* 
 * Quotient Filter
 * This serial version implementation is modified from implementation by Vedant Kumar (under MIT License).
 * https://github.com/vedantk/quotient-filter/
 **/

#include"ParallelQFWNOrder.h"
#include"helperFunctions.h"
#include"helperFunctionsQF.h"
#include<cstdlib>
#include<ctime>
#include<cstring>
#include<cstdio>
#include<inttypes.h>

#define LOW_MASK(n) ((1ULL << n) - 1ULL)

ParallelQFWNOrder::
ParallelQFWNOrder(const size_t q,
	const size_t r,
	const int _tn) {
	//generate random seed for the hash function of quotient filter
	std::srand(std::time(nullptr));
	seed = (uint32_t)std::rand();

	qbits = q;
	rbits = r;
	elem_bits = rbits + 3;
	qmask = LOW_MASK(q);
	rmask = LOW_MASK(r);
	elem_mask = LOW_MASK(elem_bits);
	table = (uint64_t *) calloc(table_size(qbits, rbits), 1);	

	omp_set_num_threads(_tn);

	/*
	 * It is highly likely that all runs have length O(log m) ~ = qbits
	 * where m = 2^qbits is the number of slots in the table.
	 * But, when occupancy rate is high (>75%), this doesn't hold.
	 *
	 * So, our strategy here is to keep a variable lock range. 
	 * When rough estimated cluster length > lock_block_size / 2 , double lock_block_size.
	 * and if length > lock_block_size, give a waning of possible false negative due to race conditions.
	 * (though it is almost impossible with the above strategy and good hashing function.)
	 *
	 */
	lock_block_length = qbits * 3;
	lock_size = ((uint64_t)1 << q) % lock_block_length == 0? ((uint64_t)1 << q) / lock_block_length: ((uint64_t)1 << q) / lock_block_length + 1;
	lock = (omp_nest_lock_t *) calloc(lock_size, sizeof(omp_nest_lock_t));

	#pragma omp parallel for
    for (uint64_t i=0; i<lock_size; i++)
    	omp_init_nest_lock(&(lock[i]));
}

ParallelQFWNOrder::
~ParallelQFWNOrder() {
	free(table);

	#pragma omp parallel for
    for (uint64_t i=0; i<lock_size; i++)
    	omp_destroy_nest_lock(&(lock[i]));
}

/* Return QF[index] in the lower bits. */
uint64_t ParallelQFWNOrder::
get_element(uint64_t index) {
	uint64_t element = 0;
	size_t bit_pos = elem_bits * index;
	size_t tab_pos = bit_pos / 64;
	size_t slot_pos = bit_pos % 64;
	int spill_bits = (slot_pos + elem_bits) - 64;
	element = (table[tab_pos] >> slot_pos) & elem_mask;
	if (spill_bits > 0) {
		++tab_pos;
		uint64_t x = table[tab_pos] & LOW_MASK(spill_bits);
		element |= x << (elem_bits - spill_bits);
	}
	return element;
}

/* Set the lower bits of element into QF[index] */
void ParallelQFWNOrder::
set_element(uint64_t index, 
			uint64_t element) {
	size_t bit_pos = elem_bits * index;
	size_t tab_pos = bit_pos / 64;
	size_t slot_pos = bit_pos % 64;
	int spill_bits = (slot_pos + elem_bits) - 64;
	element &= elem_mask;
	table[tab_pos] &= ~(elem_mask << slot_pos);
	table[tab_pos] |= element << slot_pos;
	if (spill_bits > 0) {
		++tab_pos;
		table[tab_pos] &= ~LOW_MASK(spill_bits);
		table[tab_pos] |= element >> (elem_bits - spill_bits);
	}
}

/* Find the start index of the run for the quotient of hashed fingerprint (fq). */
uint64_t ParallelQFWNOrder::
find_run_index(uint64_t fq) {
	/* Find the start of the cluster. */
	uint64_t b = fq;
	while (is_shifted(get_element(b))) {
		b = decrement(b, qmask);
	}

	/* Find the start of the run for fq. */
	uint64_t s = b;
	while (b != fq) {
		do {
			s = increment(s, qmask);
		} while (is_continuation(get_element(s)));

		do {
			b = increment(b, qmask);
		} while (!is_occupied(get_element(b)));
	}
	return s;
}

/* Insert element into QF[index], shifting elements as necessary. */
uint64_t ParallelQFWNOrder::
insert_into(uint64_t index, uint64_t element) {
	uint64_t prev;
	uint64_t curr = element;
	bool empty;

	do {
		prev = get_element(index);
		empty = is_empty_element(prev);
		if (!empty) {
			prev = set_shifted(prev);
			if (is_occupied(prev)) {
				curr = set_occupied(curr);
				prev = clr_occupied(prev);
			}
		}
		set_element(index, curr);
		curr = prev;
		index = increment(index, qmask);
	} while (!empty);

	return index; // for calculating run length
}

void ParallelQFWNOrder::
add(const void* key,
	const int len) {
	uint64_t hash_val = hash(key, len, seed);
	uint64_t fq = hash_to_quotient(hash_val, rbits, qmask);
	uint64_t fr = hash_to_remainder(hash_val, rmask);

	//lock according to table index fq
	uint64_t lock_index = fq / lock_block_length;

	if(lock_index!=0)
		omp_set_nest_lock(&(lock[lock_index-1]));
	
	omp_set_nest_lock(&(lock[lock_index]));
	
	if(lock_index!=lock_size-1)
		omp_set_nest_lock(&(lock[lock_index+1]));

	uint64_t T_entry_fq = get_element(fq);
	uint64_t entry = (fr << 3) & ~7;

	if (is_empty_element(T_entry_fq)) {
		set_element(fq, set_occupied(entry));
	} else {
		bool canonical_occupied = is_occupied(T_entry_fq);
		if (!canonical_occupied) {
			set_element(fq, set_occupied(T_entry_fq));
		}

		uint64_t start = find_run_index(fq);
		uint64_t s = start;
		
		bool duplicated = false;

		if (canonical_occupied) {
			/* Move the cursor to the insert position in the fq run. */
			do {
				uint64_t reminder = get_remainder(get_element(s));
				if (reminder == fr) {
					duplicated = true; //duplicate element
					#ifdef DEBUG
						printf("Hard collision. Hash value: %" PRIu64 " fq: %" PRIu64 " fr: %" PRIu64 " \n", hash_val, fq, fr);
					#endif
					break;
				} else if (reminder > fr) {
					break;
				}
				s = increment(s, qmask);
			} while (is_continuation(get_element(s)));

			if(!duplicated) {
				if (s == start) {
					/* The old start-of-run becomes a continuation. */
					uint64_t old_head = get_element(start);
					set_element(start, set_continuation(old_head));
				} else {
					/* The new element becomes a continuation. */
					entry = set_continuation(entry);
				}
			}
		}

		if(!duplicated) {
			/* Set the shifted bit if we can't use the canonical slot. */
			if (s != fq) {
				entry = set_shifted(entry);
			}

			uint64_t stop_index = insert_into(s, entry);

			//very rough and conservative estimate (not true value) of the cluster length by run length. 
			//ignore those cases stop_index > start, we should have enough samples.
			uint64_t length = stop_index > start? 2 * (stop_index - start) : 0; 

			//if run length > lock_block_length, give a thread safety warning.
			if(length / 2 > lock_block_length) {
				printf("Warning: Cluster length(%lu) might be larger than lock covered length(%d). It might produce false negative query result.\n", length, lock_block_length); 
			}
			if(length > lock_block_length) {
				#pragma omp critical
				if(length > lock_block_length) {
					lock_block_length *= 2; //double the lock range to avoid race conditions
					#ifdef DEBUG
						printf("Covered length by a lock: %d\n", lock_block_length);
					#endif 
				}
			}
		}
	}
	
	if(lock_index!=lock_size-1)
		omp_unset_nest_lock(&(lock[lock_index+1]));
	
	omp_unset_nest_lock(&(lock[lock_index]));

	if(lock_index!=0)
		omp_unset_nest_lock(&(lock[lock_index-1]));
}

bool ParallelQFWNOrder::
query(const void* key,
	const int len) {
	uint64_t hash_val = hash(key, len, seed);
	uint64_t fq = hash_to_quotient(hash_val, rbits, qmask);
	uint64_t fr = hash_to_remainder(hash_val, rmask);
	uint64_t T_entry_fq = get_element(fq);

	/* If this quotient has no run, give up. */
	if (!is_occupied(T_entry_fq)) {
		return false;
	}

	/* Scan the sorted run for the target remainder. */
	uint64_t s = find_run_index(fq);
	do {
		uint64_t reminder = get_remainder(get_element(s));
		if (reminder == fr) {
			return true;
		} else if (reminder > fr) {
			return false;
		}
		s = increment(s, qmask);
	} while (is_continuation(get_element(s)));
	return false;

	return true;
}

void ParallelQFWNOrder::
add_batch(const void* keys,
	const int batchLen,
	const int keyLen) {
	const BYTE* keyArr = (const BYTE*)keys;

	#pragma omp parallel for
	for (int i = 0; i < batchLen; ++i) {
		add(&keyArr[i * keyLen], keyLen);
	}
}

void ParallelQFWNOrder::
query_batch(const void* keys,
	BYTE* results,
	const int batchLen,
	const int keyLen) {
	const BYTE* keyArr = (const BYTE*)keys;

	#pragma omp parallel for
	for (int i = 0; i < batchLen; ++i) {
		if (query(&keyArr[i*keyLen], keyLen)) {
			setBit(results, i);
		}
	}


}

