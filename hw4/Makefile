all:
	gcc -Wall -g -std=c99 -D_POSIX_C_SOURCE main.c -o smallsh

format:
	clang-format -i *.c
clean:
	@rm -rf *.o *.rooms.*
	@rm -f stulag.buildrooms
	@rm -f currentTime.txt
	@rm -f stulag.adventure
