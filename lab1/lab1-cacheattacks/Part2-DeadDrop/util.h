
// You may only use fgets() to pull input from stdin
// You may use any print function to stdout to print 
// out chat messages
#include <stdio.h>

// You may use memory allocators and helper functions 
// (e.g., rand()).  You may not use system().
#include <stdlib.h>

#include <inttypes.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>

#ifndef UTIL_H_
#define UTIL_H_

#define ADDR_PTR uint64_t 
#define CYCLES uint32_t

#define CHANNEL_DEFAULT_INTERVAL        0x00080000
#define CHANNEL_SYNC_TIMEMASK           0x000FFFFF
#define CHANNEL_SYNC_JITTER             0x0100

#define LINE_SIZE 64
#define L2_CACHE_SIZE 1048576
#define L2_ASSOCIATIVITY 16
#define L2_NUM_SETS (L2_CACHE_SIZE / (LINE_SIZE * L2_ASSOCIATIVITY))
#define NUM_OFFSET_BITS 6
#define NUM_INDEX_BITS 10
#define NUM_OFF_IND_BITS (NUM_OFFSET_BITS + NUM_INDEX_BITS)


#define EVIC_SIZE (2 * L2_ASSOCIATIVITY)
#define BUFF_SIZE (1<<22)
#define MAX_BUFFER_LEN	1024

struct config {
	int interval;
    uintptr_t *eviction_set;
    void *map_buff;
};

CYCLES get_time();
CYCLES cc_sync();

CYCLES measure_one_block_access_time(ADDR_PTR addr);
uint64_t get_l2_cache_index(ADDR_PTR addr);

// You Should Not Use clflush in your final submission
// It is only used for debug
void clflush(ADDR_PTR addr);

char *string_to_binary(char *s);
char *binary_to_string(char *data);

int string_to_int(char* s);

void init_config(struct config *config, int argc, char **argv);
void prime_L2_set(struct config *config);

void free_set(struct config *config);


#endif
