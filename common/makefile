all:test_all

test_all:test_all.c HeapSort.* MemoryMgr.*
	gcc $^ -lpthread -o $@ -g

clean:
	rm test_all
