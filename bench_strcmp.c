/*
 * String Comparison Benchmarks
 * ============================
 *
 * - Various strcmp methods
 * - Using RTDSC timer
 * - -msse didn't show any diff on platforms 1 and 2
 * 
 * Platforms
 * ---------
 *
 * 1.
 * Linux Ubuntu 18 - GCC 7.3.0 - Intel(R) Core(TM) i7-8550U CPU @ 1.8GHz
 * gcc bench_strcmp.c -O3
 *
 * 2.
 * MacOSX 10.14 - Clang 1000.11.45 - Intel(R) Core(TM) i5-5257U @ 2.70GHz
 * clang bench_strcmp.c -O3 
 *
 * Results
 * -------
 *
 *  Platform | strcmp | strcmp with prefix | hash runtime | hash ahead of time
 * ==========|========|====================|==============|===================
 *  1.       | 1318   | 359                | 1838         | 140
 *  2.       | 32715  | 475                | 2640         | 243 
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <x86intrin.h>

const char *strings[] = {
	"a", "b", "c",
	"1", "2", "3",
	"abc", "123",

	"if", "else if", "else", "break", "continue", "for", "while", "do",
	"goto",	"struct", "int", "float", "unsigned", "double", "char", "const",
	"cpu", "gpu", "memory", "keyboard", "screen", "mouse", "template",
	"compiler", "type", "class", "jaffa cake", "then", "reduce", "reuse",
	"recycle", "black cats", "kiteboard", "surfboard", "skateboard",
	"wakeboard", "wobbleboard", "breadboard",

	"A really long string that takes up space",
	"This is also a longer string that takes up space, time, and sugar",
	"Everybody jump jump! Everybody jump jump jump jump jump jump!",
	"Flowers with purple spots, bannanas and apples",

	"needle",
	
	NULL
};

const char *search_for = "needle";

uint64_t
hash_str(const char *str) {
	uint64_t hash = 5381;
	int c;
	while(c = *str++, c) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}


uint64_t
get_time_rdtsc() {
	return __builtin_ia32_rdtsc();	
}


/* A basic string compare function */
uint64_t
bench_strcmp() {
	const char **str_it = &strings[0];
	uint64_t start = get_time_rdtsc();

	while(*str_it) {
		if(strcmp(*str_it, search_for) == 0) {
			break;
		}
		++str_it;
	}

	uint64_t end = get_time_rdtsc();

	printf("Found %s\n", *str_it);
	
	return end - start;
}


/* Same as above but check the `char` before
   running the strcmp */
int
pre_check(uint8_t *a, uint8_t *b) {
	return *a == *b;
}


uint64_t
bench_strcmp_prefix() {
	const char **str_it = &strings[0];
	uint64_t start = get_time_rdtsc();

	uint8_t *search_int = (uint8_t*)search_for;

	const char *str;
	while(str = *str_it++, str) {
		/* check prefix first */
		if(!pre_check((uint8_t*)*str_it, search_int)) {
			continue;
		}

		/* compare whole string */
		if(strcmp(*str_it, search_for) == 0){
			break;
		}
	}

	uint64_t end = get_time_rdtsc();

	printf("Found %s\n", *str_it);

	return end - start;
}


/* hashing strings as we go */
uint64_t
bench_hash_rt() {
	const char **str_it = &strings[0];
	uint64_t start = get_time_rdtsc();
	uint64_t search_hash = hash_str(search_for);

	while(*str_it) {
		uint64_t hash = hash_str(*str_it);
		if(hash == search_hash) {
			break;
		}
		++str_it;
	}

	uint64_t end = get_time_rdtsc();
	
	printf("Found %s\n", *str_it);
	
	return end - start;
}


/* hashing everything ahead of time */
uint64_t
bench_hash_at() {
	/* build hash table */
	uint64_t hash_arr[(sizeof(strings) / sizeof(strings[0]))];
	int count = (sizeof(strings) / sizeof(strings[0]));
	int i;
	for(i = 0; i < count; ++i) {
		if(strings[i] == NULL) {
			hash_arr[i] = (uint64_t)-1;
		}
		else {
			hash_arr[i] = hash_str(strings[i]);
		}
	}

	/* search */
	uint64_t *hash_it = &hash_arr[0];
	uint64_t search_hash = hash_str(search_for);

	uint64_t start = get_time_rdtsc();
	
	while(*hash_it != (uint64_t)-1) {
		if(*hash_it == search_hash) {
			break;
		}

		++hash_it;
	}

	uint64_t end = get_time_rdtsc();
	
	printf("Found %s\n", strings[hash_it - &hash_arr[0]]);
	
	return end - start;
}


/* Benchmark */
int
main() {
	printf("strcmp: %llu\n--\n", bench_strcmp());
	printf("strcmp with prefix: %llu\n--\n", bench_strcmp_prefix());
	printf("hash rt: %llu\n--\n", bench_hash_rt());
	printf("hash at: %llu\n--\n", bench_hash_at());

	return 0;
}
