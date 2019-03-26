/*
 * Error Checking Benchmarks
 * =========================
 * 
 * When writing an API error checking often happens, alot of the time its
 * trivial. Quick bench to see what is faster, branches or bit ops.
 * 
 * - Error checking with Table vs Branches
 * - Using RTDSC timer
 *
 * Platforms
 * ---------
 *
 * 1.
 * Linux Ubuntu 18 - GCC 7.3.0 - Intel(R) Core(TM) i7-8550U CPU @ 1.8GHz
 * gcc bench_err_check.c -O3
 * clang bench_err_check.c -O3
 * 2.
 * MacOSX 10.14 - Clang 1000.11.45 - Intel(R) Core(TM) i5-5257U @ 2.70GHz
 * clang bench_err_check.c -O3 
 *
 * Mixed Input Results
 * -------------------
 *
 *  Platform | Branches | Unlikely | Giant | Branch Tree | Error Table 
 * ==========|==========|==========|=======|=============|============
 *  1(gcc)   | 399      | 409      | 425   | 318         | 533         
 *  1(clang) | 717      | 423      | 381   | 593         | 291
 *  2        | 660      | 564      | 636   | 837         | 375         
 *
 * Valid Input Results
 * -------------------
 *
 *  Platform | Branches | Unlikely | Giant | Branch Tree | Error Table 
 * ==========|==========|==========|=======|=============|============
 *  1(gcc)   | 174      | 170      | 338   | 136         | 334         
 *  1(clang) | 338      | 158      | 184   | 325         | 275
 *  2        | 369      | 306      | 258   | 432         | 357         
 *
 */

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
};

uint64_t mixed_input_count = (sizeof(mixed_inputs) / sizeof(mixed_inputs[0]));

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
};

uint64_t valid_input_count = (sizeof(valid_inputs) / sizeof(valid_inputs[0]));


#define likely(x)       __builtin_expect((x),1)
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


/* checks inputs using hints */
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



/* Benchmark */
int
main() {
  printf("Mixed Inputs\n");
  printf("============\n");
  printf("branches: %llu\n--\n", bench_error_branches(mixed_inputs, mixed_input_count));
  printf("unlikely branches: %llu\n--\n", bench_error_unlikely_branches(mixed_inputs, mixed_input_count));
  printf("giant check: %llu\n--\n", bench_error_giant_check(mixed_inputs, mixed_input_count));
  printf("branch tree: %llu\n--\n", bench_error_branch_tree(mixed_inputs, mixed_input_count));
  printf("Error Table: %llu\n--\n", bench_error_table(mixed_inputs, mixed_input_count));
  
  printf("\nValid Inputs\n");
  printf("============\n");
  printf("branches: %llu\n--\n", bench_error_branches(valid_inputs, valid_input_count));
  printf("unlikely branches: %llu\n--\n", bench_error_unlikely_branches(valid_inputs, valid_input_count));
  printf("giant check: %llu\n--\n", bench_error_giant_check(valid_inputs, valid_input_count));
  printf("branch tree: %llu\n--\n", bench_error_branch_tree(valid_inputs, valid_input_count));
  printf("Error Table: %llu\n--\n", bench_error_table(valid_inputs, valid_input_count));

  return 0;
}