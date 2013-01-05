LIB=-lslog
DEBUG=-g -D__SLOG_FULL__

LIB_NAME=libnetframework.a
SO_NAME=libnetframework.so

LIB_INC=/usr/local/include/libnetframework
LIB_PATH=/usr/local/lib

BIN_PATH=/usr/local/bin

lib:static share

install:
	mkdir ${LIB_INC} -p
	cp *.h ${LIB_INC}
	cp ${LIB_NAME} ${LIB_PATH}
	ln -sf ${LIB_PATH}/${LIB_NAME} /usr/lib/${LIB_NAME}
	cp ${SO_NAME} ${LIB_PATH}
	ln -sf ${LIB_PATH}/${SO_NAME} /usr/lib/${SO_NAME}
	mkdir ${LIB_INC}/AppTemplate -p
	cp AppTemplate/*.h ${LIB_INC}/AppTemplate
	cp AppTemplate/*.cpp ${LIB_INC}/AppTemplate
	cp AppTemplate/enetlib ${BIN_PATH}

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
	rm *.gch *.o *.a *.so -rf

