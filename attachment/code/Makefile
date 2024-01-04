
CC = gcc
CFLAGS = -g

.PHONY:  clean

all:  sim

sim:  main.o cache.o
	$(CC) -o sim main.o cache.o -lm #参数"-lm"代表链接到数学库。GCC编译器默认不会链接到数学库，因此在使用数学函数时（如sqrt、sin、cos等），为了避免出现链接错误，需要手动添加"-lm"参数。

main.o:  main.c cache.h
	$(CC) $(CFLAGS) -c main.c

cache.o:  cache.c cache.h
	$(CC) $(CFLAGS) -c cache.c

clean:
	rm sim *.o