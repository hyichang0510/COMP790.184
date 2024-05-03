
#include "util.h"

/* Measure the time it takes to access a block with virtual address addr. */
CYCLES measure_one_block_access_time(ADDR_PTR addr)
{
	CYCLES cycles;

	asm volatile("mov %1, %%r8\n\t"
	"lfence\n\t"
	"rdtsc\n\t"
	"mov %%eax, %%edi\n\t"
	"mov (%%r8), %%r8\n\t"
	"lfence\n\t"
	"rdtsc\n\t"
	"sub %%edi, %%eax\n\t"
	: "=a"(cycles) /*output*/
	: "r"(addr)
	: "r8", "edi");	

	return cycles;
}

// CYCLES measure_one_block_access_time(ADDR_PTR addr)
// {
//     uint64_t cycles;

//     asm volatile("mov %1, %%r8\n\t"
//     "mfence\n\t"
//     "lfence\n\t"
//     "rdtscp\n\t"
//     "mov %%eax, %%edi\n\t"
//     "mfence\n\t"
//     "mov (%%r8), %%r8\n\t"
//     "mfence\n\t"
//     "lfence\n\t"
//     "rdtscp\n\t"
//     "mfence\n\t"
//     "lfence\n\t"
//     "sub %%edi, %%eax\n\t"
//     : "=a"(cycles) /*output*/
//     : "r"(addr)    /*input*/
//     : "r8", "edi"); /*reserved register*/

//     return cycles;
// }

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
