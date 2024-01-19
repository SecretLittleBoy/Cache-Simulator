
CC = gcc
CFLAGS = -g

.PHONY:  clean test

all:  sim
	rm *.o

test: all
	cd ../ && ./runPublic
	diff ../public-assoc1.log ../outputs/public-assoc1.out
	diff ../public-block1.log ../outputs/public-block1.out
	diff ../public-instr1.log ../outputs/public-instr1.out
	diff ../public-write1.log ../outputs/public-write1.out
	diff ../spice.log ../outputs/spice1.out
	diff ../spice10.log ../outputs/spice10.out
	diff ../spice100.log ../outputs/spice100.out
	diff ../spice1000.log ../outputs/spice1000.out
	echo "Passed all tests!"

sim:  main.o cache.o
	$(CC) -o sim main.o cache.o -lm #参数"-lm"代表链接到数学库。GCC编译器默认不会链接到数学库，因此在使用数学函数时（如sqrt、sin、cos等），为了避免出现链接错误，需要手动添加"-lm"参数。

main.o:  main.c cache.h
	$(CC) $(CFLAGS) -c main.c

cache.o:  cache.c cache.h debug.h
	$(CC) $(CFLAGS) -c cache.c

clean:
	rm sim *.o