INC=-Iinc/
INC2=-Isrc/
CC=gcc

all: memshare test examples

# memshare
memshare: out/libmemshare.a out/memsend out/memwatch
src/memshare.o: src/memshare.c
	$(CC) -o $@ -shared -g -c $(INC) $<
src/queue.o: src/queue.c
	$(CC) -o $@ -shared -g -c $<

out/libmemshare.a: src/memshare.o src/queue.o
	ar -rcs $@ src/memshare.o src/queue.o

out/memsend: src/memsend.c
	$(CC) -g $(INC) -o $@ $< out/libmemshare.a -lpthread

out/memwatch: src/memwatch.c
	$(CC) -g $(INC) -o $@ $< out/libmemshare.a -lpthread


# test code
test: memshare out/reply out/main out/listen
out/reply: src/test/reply.c
	$(CC) -g $(INC) -o $@ $< out/libmemshare.a -lpthread

out/main: src/test/main.c
	$(CC) -g $(INC) -o $@ $< out/libmemshare.a -lpthread

out/listen: src/test/listen.c
	$(CC) -g $(INC) -o $@ $< out/libmemshare.a -lpthread


# example code
examples: memshare out/tlog.a out/log_test
src/examples/tlog.o: src/examples/tlog.c
	$(CC) -o $@ -shared -g -c $(INC) $(INC2) $< out/libmemshare.a

out/tlog.a: src/examples/tlog.o
	ar -rcs $@ src/examples/tlog.o

out/log_test: src/examples/main.c
	$(CC) -g $(INC) -o $@ $< out/tlog.a out/libmemshare.a -lpthread


clean:
	rm -f src/*.o
	rm -f src/examples/*.o
	rm -f out/*


