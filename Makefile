
board_of_trade:
	gcc -Iinclude/rbtree -c include/rbtree/rbtree.c test.c -g
	gcc test.o rbtree.o -o board_of_trade -g
clean:
	rm -f *.o board_of_trade