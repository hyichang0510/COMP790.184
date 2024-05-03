include ../cpu.mk

ifndef SENDER_CPU
$(error SENDER_CPU is not set)
endif

ifndef RECEIVER_CPU
$(error RECEIVER_CPU is not set)
endif

all: main

run: main
	@taskset -c $(SENDER_CPU) ./main

run-reference: reference
	@taskset -c $(SENDER_CPU) ./reference

main: main.c Makefile
	@gcc main.c -o main

.PHONY: clean

clean:
	rm -f main
