LIB=-lslog
DEBUG=-g #-D__SLOG_FULL__

LIB_NAME=libnetframework.a
SO_NAME=libnetframework.so

LIB_INC=/usr/local/include/libnetframework
LIB_PATH=/usr/local/lib
	
lib:static share
install:
	mkdir ${LIB_INC} -p
	cp *.h ${LIB_INC}
	cp  ${LIB_NAME} ${LIB_PATH}
	cp ${SO_NAME} ${LIB_PATH}
uninstall:
	rm ${LIB_INC} -r
	rm ${LIB_PATH}/${LIB_NAME}
	rm ${LIB_PATH}/${SO_NAME}
	
static:*.cpp *.h
	g++ $^ -c ${DEBUG}
	ar rc ${LIB_NAME} *.o

share:*.cpp *.h
	g++ -fPIC -c  $^ ${LIB} ${DEBUG}
	g++ -shared *.o -o libnetframework.so

clean:
	rm *.gch *.o

