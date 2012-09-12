INC_PATH=/usr/local/include
LIB_PATH=/usr/local/lib

FULL_LOG=-D__SLOG_FULL__
#FULL_LOG=


lib:slog.h slog.c
	gcc -c slog.h slog.c
	ar rc libslog.a slog.o

test_slog:slog_test.c slog.c slog.h
	gcc $^ ${FULL_LOG} -g -lpthread -o $@

install:
	cp slog.h ${INC_PATH}
	cp libslog.a ${LIB_PATH}

uninstall:
	rm ${INC_PATH}/slog.h
	rm ${LIB_PATH}/libslog.a

clean:
	rm *.o *.gch test_slog
