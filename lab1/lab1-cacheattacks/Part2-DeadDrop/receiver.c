
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
// #include <sys/mman.h>
int CACHE_MISS_LATENCY = 125;


bool detect_bit(struct config *config)
{
	int misses = 0;
	int hits = 0;
    volatile int temp;

	// Sync with sender
	CYCLES start_t = cc_sync();
	while ((get_time() - start_t) < config->interval) {
		for (int i = L2_ASSOCIATIVITY - 1; i >= 0 ; i--) {
			if(measure_one_block_access_time(config->eviction_set[i]) > CACHE_MISS_LATENCY) misses++;
			else hits++;
		}
	}

	//tried many times to get hard code to prevent pseudo cache miss 
	return ((misses >= 1.6 * hits) && (hits > 15) || (misses > 900 && (hits > 15)));
}

int main(int argc, char **argv)
{
	// Put your covert channel setup code here
	struct config config;
	init_config(&config, argc, argv);
	char msg_ch[MAX_BUFFER_LEN + 1];

	uint32_t bitSequence = 0;
	uint32_t sequenceMask = ((uint32_t) 1<<8) - 1;
	uint32_t expSequence = 0b10101010;


	printf("Receiver now listening...\n");
	
	prime_L2_set(&config);

	while (1) {
	    prime_L2_set(&config);

		bool bitReceived = detect_bit(&config);

		// Detect the sequence '10101010' that indicates sender is sending a message	
		bitSequence = ((uint32_t) bitSequence<<1) | bitReceived;
		if ((bitSequence & sequenceMask) == expSequence) {
			int binary_msg_len = 0;
			int strike_zeros = 0;
			for (int i = 0; i < MAX_BUFFER_LEN; i++) {
				binary_msg_len++;

				if (detect_bit(&config)) {
					msg_ch[i] = '1';
					strike_zeros = 0;
				} else {
					msg_ch[i] = '0';
					if (++strike_zeros >= 8 && i % 8 == 0) {
						break;
					}
				}
			}
			msg_ch[binary_msg_len - 8] = '\0';

			printf("> %s\n", binary_to_string(msg_ch));
	
			// Terminate loop if received "exit" message
			if (strcmp(binary_to_string(msg_ch), "exit") == 0) {
				break;
			}
		}

		// Add a clflush to clean the cache after iteration seems can improve the rate of sucessful transit, don't know why
		// free_set(&config);
	}

	printf("Receiver finished.\n");

	return 0;
}


