CC = arm-none-eabi-gcc

CFLAGS = -c -mcpu=cortex-m4 -mthumb -Wall -O0

all: startup.o main.o

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o