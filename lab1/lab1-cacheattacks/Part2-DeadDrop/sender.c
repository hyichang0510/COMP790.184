
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
// #include <sys/mman.h>

// TODO: define your own buffer size
// #define BUFF_SIZE (1<<21)
//#define BUFF_SIZE [TODO]

void send_bit(bool one, struct config *config)
{
    // Synchronize with receiver
    CYCLES start_t = cc_sync();
    if (one) {
        // Repeatedly access eviction set
        while ((get_time() - start_t) < config->interval) {
            prime_L2_set(config);
        }
    } else {
        // Do Nothing
        while (get_time() - start_t < config->interval) {}
    }
}

int main(int argc, char **argv)
{
  struct config config;

  //prime the specific L2 cache set index 
  init_config(&config, argc, argv);
  bool sequence[8] = {1,0,1,0,1,0,1,0};


  printf("Please type a message.\n");

  int sending = 1;
  while (sending) {
    char text_buf[128];    
    printf("< ");
    fgets(text_buf, sizeof(text_buf), stdin);
    if (strcmp(text_buf, "exit\n") == 0) {
        sending = 0;
    }

      // // Convert that message to binary
    char *msg = string_to_binary(text_buf);

      // Send a '1010101010' bit sequence tell the receiver
    for (int i = 0; i < 8; i++) {
        send_bit(sequence[i], &config);
    }


    // Send the message bit by bit
    size_t msg_len = strlen(msg);
    for (int ind = 0; ind < msg_len; ind++) {
        if (msg[ind] == '0') {
            send_bit(false, &config);
        } else {
            send_bit(true, &config);
        }
    }

    // free_set(&config);
  }

  printf("Sender finished.\n");
  return 0;
}


