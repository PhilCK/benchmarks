/*
 * Error Checking Benchmarks
 * =========================
 * 
 * When writing an API error checking often happens, alot of the time its
 * trivial. Quick bench to see what is faster, branches or bit ops.
 * 
 * - Error checking with Table vs Branches
 * - Using RTDSC timer
 * - Sample size needs to bigger
 * - Maybe should run these tests indiviudally
 *
 * Platforms
 * ---------
 *
 * 1.
 * Linux Ubuntu 18 - Intel(R) Core(TM) i7-8550U CPU @ 1.8GHz
 * gcc bench_err_check.c -O3 (GCC 7.3.0)
 * clang bench_err_check.c -O3 (Clang 6, 7)
 *
 * 2.
 * MacOSX 10.14 - Intel(R) Core(TM) i5-5257U @ 2.70GHz
 * clang bench_err_check.c -O3 (Clang 1000.11.45)
 *
 * 3.
 * Linux Ubuntu 16 - Intel(R) Core(TM) i7-6700K CPU @ 4.00GHz
 * gcc bench_strcmp.c -DBENCH_TO_RUN=BENCH_<TEST> -O3 (GCC 5.4.0)
 * clang bench_strcmp.c -DBENCH_TO_RUN=BENCH_<TEST> -O3 (Clang 7.0.0)
 *
 * Mixed Input Results
 * -------------------
 * 
 * _Note:_ Brackets indicate time with branch prediction
 * _Note:_ Times are fastest run
 *
 *  Platform | Branches | Giant    | Branch Tree | Error Table | No Check
 * ==========|==========|==========|=============|=============|==========
 *  3(Clang) | 1183(795)| 714(732) | 1266(855)   | 697         | 352
 *  3(GCC)   | 789(624) | 672(737) | 732(602)    | 686         | 320 
 *  
 * Valid Input Results
 * -------------------
 *
 * _Note:_ Brackets indicate time with branch prediction
 * _Note:_ Times are fastest run
 *
 *  Platform | Branches | Giant    | Branch Tree | Error Table | No Check
 * ==========|==========|==========|=============|=============|==========
 *  3(Clang) | 839(459) | 417(406) | 835(416)    | 720         | 346
 *  3(GCC)   | 388(346) | 435(410) | 305(356)    | 680         | 334 
 *
 */

#define BENCH_BRANCHES 1
#define BENCH_BRANCHES_HINTS 2
#define BENCH_GIANT 3
#define BENCH_GIANT_HINTS 4
#define BENCH_TREE 5
#define BENCH_TREE_HINTS 6
#define BENCH_TABLE 7
#define BENCH_NONE 8


#define BENCH_MIXED_INPUTS 1
#define BENCH_VALID_INPUTS 2


#ifndef BENCH_TO_RUN
#define BENCH_TO_RUN BENCH_BRANCHES
#endif

#ifndef BENCH_INPUTS
#define BENCH_INPUTS BENCH_MIXED_INPUTS
#endif


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <x86intrin.h>


/* Benchmark checks this that the envelope is valid */
/* top left corner must be less than bottom right corner */
struct env {
        int top_left_x, top_left_y;
        int bot_right_x, bot_right_y;
};

/* input data */

struct env mixed_inputs[] = {
        {0, 0, 10, 10},     /* valid */
        {10, 10, 0, 0},     /* invalid - bot right less than top left */
        {30, 30, 40, 40},   /* valid */
        {30, 30, 20, 40},   /* invalid - bot right x */
        {50, 50, 90, 90},   /* valid */
        {30, 30, 40, 10},   /* invalid - bot right y */
        {40, 40, 50, 60},   /* valid */
        {50, 50, 200, 90},  /* invalid - bot right max x*/
        {3, 4, 5, 5},       /* valid */
        {4, 4, 5, 200},     /* invalid - bot right max y */
        {10, 10, 13, 13},   /* valid */
        {20, 200, 30, 300}, /* invalid - top left max y */
        {50, 30, 60, 70},   /* valid */
        {200, 20, 300, 30}, /* invalid - top left max x */
        {0, 0, 10, 10},     /* valid */
        {10, 10, 0, 0},     /* invalid - bot right less than top left */
        {30, 30, 40, 40},   /* valid */
        {30, 30, 20, 40},   /* invalid - bot right x */
        {50, 50, 90, 90},   /* valid */
        {30, 30, 40, 10},   /* invalid - bot right y */
        {40, 40, 50, 60},   /* valid */
        {50, 50, 200, 90},  /* invalid - bot right max x*/
        {3, 4, 5, 5},       /* valid */
        {4, 4, 5, 200},     /* invalid - bot right max y */
        {10, 10, 13, 13},   /* valid */
        {20, 200, 30, 300}, /* invalid - top left max y */
        {50, 30, 60, 70},   /* valid */
        {200, 20, 300, 30}, /* invalid - top left max x */
        {0, 0, 10, 10},     /* valid */
        {10, 10, 0, 0},     /* invalid - bot right less than top left */
        {30, 30, 40, 40},   /* valid */
        {30, 30, 20, 40},   /* invalid - bot right x */
        {50, 50, 90, 90},   /* valid */
        {30, 30, 40, 10},   /* invalid - bot right y */
        {40, 40, 50, 60},   /* valid */
        {50, 50, 200, 90},  /* invalid - bot right max x*/
        {3, 4, 5, 5},       /* valid */
        {4, 4, 5, 200},     /* invalid - bot right max y */
        {10, 10, 13, 13},   /* valid */
        {20, 200, 30, 300}, /* invalid - top left max y */
        {50, 30, 60, 70},   /* valid */
        {200, 20, 300, 30}, /* invalid - top left max x */
        {0, 0, 10, 10},     /* valid */
        {10, 10, 0, 0},     /* invalid - bot right less than top left */
        {30, 30, 40, 40},   /* valid */
        {30, 30, 20, 40},   /* invalid - bot right x */
        {50, 50, 90, 90},   /* valid */
        {30, 30, 40, 10},   /* invalid - bot right y */
        {40, 40, 50, 60},   /* valid */
        {50, 50, 200, 90},  /* invalid - bot right max x*/
        {3, 4, 5, 5},       /* valid */
        {4, 4, 5, 200},     /* invalid - bot right max y */
        {10, 10, 13, 13},   /* valid */
        {20, 200, 30, 300}, /* invalid - top left max y */
        {50, 30, 60, 70},   /* valid */
        {200, 20, 300, 30}, /* invalid - top left max x */
};

uint64_t mixed_inputs_count = (sizeof(mixed_inputs) / sizeof(mixed_inputs[0]));

struct env valid_inputs[] = {
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */  
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */  
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */  
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */
        {0, 0, 10, 10},     /* valid */
        {30, 30, 40, 40},   /* valid */
        {50, 50, 90, 90},   /* valid */
        {40, 40, 50, 60},   /* valid */
        {3, 4, 5, 5},       /* valid */
        {10, 10, 13, 13},   /* valid */
        {50, 30, 60, 70},   /* valid */  
};

uint64_t valid_inputs_count = (sizeof(valid_inputs) / sizeof(valid_inputs[0]));

#define unlikely(x)     __builtin_expect((x),0)

uint64_t
get_time_rdtsc() {
  return __builtin_ia32_rdtsc();  
}

/* checks inputs with indivual if statements */
uint64_t
bench_error_branches(
        struct env *inputs,
        uint64_t input_count) 
{
        uint64_t i;
        uint64_t invalid = 0;
        uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                if(inputs[i].top_left_x > inputs[i].bot_right_x) {
                        invalid += 1;
                        continue;
                }

                if(inputs[i].top_left_y > inputs[i].bot_right_y) {
                        invalid += 1;
                        continue;
                }

                if(inputs[i].top_left_x > 100) {
                        invalid += 1;
                        continue;
                }

                if(inputs[i].bot_right_x > 100) {
                        invalid += 1;
                        continue;
                }

                if(inputs[i].top_left_y > 100) {
                        invalid += 1;
                        continue;
                }

                if(inputs[i].bot_right_y > 100) {
                        invalid += 1;
                        continue;
                }

                valid += 1;
        }

  uint64_t end = get_time_rdtsc();

  printf("Valid/Invalid: %llu %llu\n", valid, invalid);
  
  return end - start;
}

/* checks inputs with indivual if statements */
uint64_t
bench_error_unlikely_branches(
        struct env *inputs,
        uint64_t input_count) 
{
        uint64_t i;
        uint64_t invalid = 0;
        uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                if(unlikely(inputs[i].top_left_x > inputs[i].bot_right_x)) {
                        invalid += 1;
                        continue;
                }

                if(unlikely(inputs[i].top_left_y > inputs[i].bot_right_y)) {
                        invalid += 1;
                        continue;
                }

                if(unlikely(inputs[i].top_left_x > 100)) {
                        invalid += 1;
                        continue;
                }

                if(unlikely(inputs[i].bot_right_x > 100)) {
                        invalid += 1;
                        continue;
                }

                if(unlikely(inputs[i].top_left_y > 100)) {
                        invalid += 1;
                        continue;
                }

                if(unlikely(inputs[i].bot_right_y > 100)) {
                        invalid += 1;
                        continue;
                }

                valid += 1;
        }

  uint64_t end = get_time_rdtsc();

  printf("Valid/Invalid: %llu %llu\n", valid, invalid);
  
  return end - start;
}


/* checks inputs with indivual if statements */
uint64_t
bench_error_giant_check(
        struct env *inputs,
        uint64_t input_count) 
{
        uint64_t i;
        uint64_t invalid = 0;
        uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                if((inputs[i].top_left_x > inputs[i].bot_right_x) ||
                   (inputs[i].top_left_y > inputs[i].bot_right_y) ||
                   (inputs[i].top_left_x > 100) ||
                   (inputs[i].bot_right_x > 100) ||
                   (inputs[i].top_left_y > 100) ||
                   (inputs[i].bot_right_y > 100)) {
                        invalid += 1;
                        continue;
                }

                valid += 1;
        }

  uint64_t end = get_time_rdtsc();

  printf("Valid/Invalid: %llu %llu\n", valid, invalid);
  
  return end - start;
}

/* checks inputs with indivual if statements */
uint64_t
bench_error_unlikely_giant_check(
        struct env *inputs,
        uint64_t input_count) 
{
        uint64_t i;
        uint64_t invalid = 0;
        uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                if(unlikely((inputs[i].top_left_x > inputs[i].bot_right_x) ||
                   (inputs[i].top_left_y > inputs[i].bot_right_y) ||
                   (inputs[i].top_left_x > 100) ||
                   (inputs[i].bot_right_x > 100) ||
                   (inputs[i].top_left_y > 100) ||
                   (inputs[i].bot_right_y > 100))) {
                        invalid += 1;
                        continue;
                }

                valid += 1;
        }

  uint64_t end = get_time_rdtsc();

  printf("Valid/Invalid: %llu %llu\n", valid, invalid);
  
  return end - start;
}

/* checks inputs with one large if/else if block */
uint64_t
bench_error_branch_tree(
        struct env *inputs,
        uint64_t input_count)
{
        uint64_t i;
        uint64_t invalid = 0;
        uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                if(inputs[i].top_left_x > inputs[i].bot_right_x) {
                        invalid += 1;
                        continue;
                }

                else if(inputs[i].top_left_y > inputs[i].bot_right_y) {
                        invalid += 1;
                        continue;
                }

                else if(inputs[i].top_left_x > 100) {
                        invalid += 1;
                        continue;
                }

                else if(inputs[i].bot_right_x > 100) {
                        invalid += 1;
                        continue;
                }

                else if(inputs[i].top_left_y > 100) {
                        invalid += 1;
                        continue;
                }

                else if(inputs[i].bot_right_y > 100) {
                        invalid += 1;
                        continue;
                }

                valid += 1;
        }

        uint64_t end = get_time_rdtsc();

        printf("Valid/Invalid: %llu %llu\n", valid, invalid);

        return end - start;
}


/* checks inputs using hints */
uint64_t
bench_error_unlikely_branch_tree(
        struct env *inputs,
        uint64_t input_count) 
{
        uint64_t i;
        uint64_t invalid = 0;
        uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                if(unlikely(inputs[i].top_left_x > inputs[i].bot_right_x)) {
                        invalid += 1;
                        continue;
                }

                else if(unlikely(inputs[i].top_left_y > inputs[i].bot_right_y)) {
                        invalid += 1;
                        continue;
                }

                else if(unlikely(inputs[i].top_left_x > 100)) {
                        invalid += 1;
                        continue;
                }

                else if(unlikely(inputs[i].bot_right_x > 100)) {
                        invalid += 1;
                        continue;
                }

                else if(unlikely(inputs[i].top_left_y > 100)) {
                        invalid += 1;
                        continue;
                }

                else if(unlikely(inputs[i].bot_right_y > 100)) {
                        invalid += 1;
                        continue;
                }

                valid += 1;
        }

        uint64_t end = get_time_rdtsc();

        printf("Valid/Invalid: %llu %llu\n", valid, invalid);

        return end - start;
}

/* checks inputs with one large if/else if block */
uint64_t
bench_error_table(
        struct env *inputs,
        uint64_t input_count)
{
        uint64_t i;
        uint64_t invalid = 0;
        uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                uint64_t err = 0;

                /* each bit represents a particular  */
                err |= (inputs[i].top_left_x > inputs[i].bot_right_x) << 1;
                err |= (inputs[i].top_left_y > inputs[i].bot_right_y) << 2;
                err |= (inputs[i].top_left_x > 100) << 3;
                err |= (inputs[i].bot_right_x > 100) << 4;          
                err |= (inputs[i].top_left_y > 100) << 5;
                err |= (inputs[i].bot_right_y > 100) << 6;

                if(err) {
                        invalid += 1;
                } else {
                        valid += 1;
                }
        }

        uint64_t end = get_time_rdtsc();

        printf("Valid/Invalid: %llu %llu\n", valid, invalid);

        return end - start;
}


/* spins over the data */
uint64_t
bench_error_no_check(
        struct env *inputs,
        uint64_t input_count)
{
        uint64_t i;
        volatile uint64_t invalid = 0;
        volatile uint64_t valid = 0;
        uint64_t start = get_time_rdtsc();

        for(i = 0; i < input_count; ++i) {
                uint64_t err = 0;

                valid += 1;
        }

        uint64_t end = get_time_rdtsc();

        printf("Valid/Invalid: %llu %llu\n", valid, invalid);

        return end - start;
}



/* Benchmark */
int
main() {
        /* input data */
        struct env *inputs = 0;
        uint64_t count = 0;

        if(BENCH_INPUTS == BENCH_MIXED_INPUTS) {
                inputs = mixed_inputs;
                count = mixed_inputs_count;
        } else if (BENCH_INPUTS == BENCH_VALID_INPUTS) {
                inputs = valid_inputs;
                count = valid_inputs_count;
        }
  
        /* benchmarks */
        if(BENCH_TO_RUN == BENCH_BRANCHES) {
                printf("branches: %llu\n--\n",
                        bench_error_branches(inputs, count));
        }

        if(BENCH_TO_RUN == BENCH_BRANCHES_HINTS) {
                printf("unlikely branches: %llu\n--\n",
                        bench_error_unlikely_branches(inputs, count));
        }

        if(BENCH_TO_RUN == BENCH_GIANT) {
                printf("giant check: %llu\n--\n",
                        bench_error_giant_check(inputs, count));

        }

        if(BENCH_TO_RUN == BENCH_GIANT_HINTS) {
                printf("unlikely giant check: %llu\n--\n",
                        bench_error_unlikely_giant_check(inputs, count));
        }
        
        if(BENCH_TO_RUN == BENCH_TREE) {
                printf("branch tree: %llu\n--\n",
                        bench_error_branch_tree(inputs, count));
        }

        if(BENCH_TO_RUN == BENCH_TREE_HINTS) {
                printf("unlikely branch tree: %llu\n--\n",
                        bench_error_unlikely_branch_tree(inputs, count));
        }

        if(BENCH_TO_RUN == BENCH_TABLE) {
                printf("Error Table: %llu\n--\n",
                        bench_error_table(inputs, count));
        }

        if(BENCH_TO_RUN == BENCH_NONE) {
                printf("No check: %llu\n--\n",
                        bench_error_no_check(inputs, count));
        }

        return 0;
}
