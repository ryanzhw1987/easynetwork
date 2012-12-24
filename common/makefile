all:test_all

test_all:test_all.c heapsort.* memorymgr.*
	gcc $^ -lpthread -o $@ -g

clean:
	rm test_all