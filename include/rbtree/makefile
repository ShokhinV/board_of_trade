CFLAGS = -g -O2

liburb.a: rbtree.o 
	ar -rc liburb.a rbtree.o

rbtree.o: rbtree.c rbtree.h rbtree_augmented.h
	gcc $(CFLAGS) -c -Wall -Werror -fpic rbtree.c

clean:
	rm -f *.o rbtest liburb.a
