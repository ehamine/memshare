INC=-Iinc/
CC=gcc

all: out/libmemshare.a out/reply out/main out/memsend

out/memshare.o: src/memshare.c
	$(CC) -o $@ -shared -c $(INC) $<
out/queue.o: src/queue.c
	$(CC) -o $@ -shared -c $<

out/libmemshare.a: out/memshare.o out/queue.o
	ar -rcs $@ out/memshare.o out/queue.o

out/reply: src/test/reply.c
	$(CC) $(INC) -o $@ $< out/libmemshare.a -lpthread

out/main: src/test/main.c
	$(CC) $(INC) -o $@ $< out/libmemshare.a -lpthread

out/memsend: src/memsend.c
	$(CC) $(INC) -o $@ $< out/libmemshare.a -lpthread

#testing: libmemshare.a
#	$(CC)

#test: proc testing

clean:
	rm out/*.o
	rm out/*.a
	rm out/*

