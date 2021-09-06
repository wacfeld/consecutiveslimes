all: consecutiveslimes.c
	clang consecutiveslimes.c -o slimechunks -pthread -O3
debug:
	make all
	gdb slimechunks
run:
	make all
	./slimechunks
clean:
	rm -rf slimechunks
