/*
 * MIT Secure Hardware Design Instructor's Solutions
 *
 * Solution provided by Alex Krastev, Spring 2022
 */

#include "util.h"

/* Measure the time it takes to access a block with virtual address addr. */
CYCLES measure_one_block_access_time(ADDR_PTR addr)
{
    CYCLES cycles;

    asm volatile("mov %1, %%r8\n\t"
    "mfence\n\t"
    "lfence\n\t"
    "rdtscp\n\t"
    "mov %%eax, %%edi\n\t"
    "mfence\n\t"
    "mov (%%r8), %%r8\n\t"
    "mfence\n\t"
    "lfence\n\t"
    "rdtscp\n\t"
    "mfence\n\t"
    "lfence\n\t"
    "sub %%edi, %%eax\n\t"
    : "=a"(cycles) /*output*/
    : "r"(addr)
    : "r8", "edi"); 

    return cycles;
}

/*
 * CLFlushes the given address.
 * 
 * Note: clflush is provided to help you debug and should not be used in your
 * final submission
 */
void clflush(ADDR_PTR addr)
{
    asm volatile ("clflush (%0)"::"r"(addr));
}

extern inline __attribute__((always_inline))
CYCLES rdtscp(void) {
	CYCLES cycles;
	asm volatile ("rdtscp"
	: /* outputs */ "=a" (cycles));

	return cycles;
}

/* 
 * Gets the value Time Stamp Counter 
 */
inline CYCLES get_time() {
    return rdtscp();
}

extern inline __attribute__((always_inline))
CYCLES cc_sync() {
    while((get_time() & CHANNEL_SYNC_TIMEMASK) > CHANNEL_SYNC_JITTER) {}
    return get_time();
}

uint64_t get_l2_cache_index(ADDR_PTR addr)
{
    uint64_t mask = ((uint64_t) 1 << 10) - 1;
    return ((uint64_t) addr >> 6) & mask;
}

/*
 * Converts a string to its binary representation.
 */
char *string_to_binary(char *s)
{
  if (s == NULL)
    return 0; /* no input string */

  size_t len = strlen(s);

  // Each char is one byte (8 bits) and + 1 at the end for null terminator
  char *binary = malloc(len * 8 + 1);
  binary[len] = '\0';

  for (size_t i = 0; i < len; ++i)
  {
    char ch = s[i];
    for (int j = 7; j >= 0; --j)
    {
      if (ch & (1 << j))
      {
        strcat(binary, "1");
      }
      else
      {
        strcat(binary, "0");
      }
    }
  }

  return binary;
}

/*
 * Converts a binary string to its ASCII representation.
 */
char *binary_to_string(char *data)
{
  // Each char is 8 bits
  size_t msg_len = strlen(data) / 8;

  // Add one for null terminator at the end
  char *msg = malloc(msg_len + 1);
  msg[msg_len] = '\0';

  for (int i = 0; i < msg_len; i++)
  {
    char tmp[8];
    int k = 0;

    for (int j = i * 8; j < ((i + 1) * 8); j++)
    {
      tmp[k++] = data[j];
    }

    msg[i] = strtol(tmp, 0, 2);
  }

  return msg;
}

/*
 * Converts a string to integer
 */
int string_to_int(char* s) 
{
  return atoi(s);
}

void init_config(struct config *config, int argc, char **argv)
{
	  config->interval = CHANNEL_DEFAULT_INTERVAL;
    config->map_buff = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    
    if (config->map_buff == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    printf("Buffer allocated at %p\n", config->map_buff);

    int i = 0;
    config->eviction_set = (uintptr_t *)malloc(sizeof(uintptr_t) * EVIC_SIZE);
    for (uintptr_t addr = (uintptr_t)config->map_buff; addr < (uintptr_t)config->map_buff + BUFF_SIZE; addr += 64) {
        if (get_l2_cache_index(addr) == 80) {
            config->eviction_set[i++] = addr;
            if(i == EVIC_SIZE) break;
        }
    }
}


void prime_L2_set(struct config *config){
    volatile int temp;
    uint64_t* L2_latency = (uint64_t *)malloc(L2_ASSOCIATIVITY * sizeof(uint64_t)); 
    for (int i = 0; i < L2_ASSOCIATIVITY; i++) {
        temp = *(volatile int *)config->eviction_set[i];
    }

}

void free_set(struct config *config){
    for(int j = 0; j < L2_ASSOCIATIVITY; j++){
        clflush(config->eviction_set[j]);
    }
}