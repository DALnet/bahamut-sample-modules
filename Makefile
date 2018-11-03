CC=gcc
RM=rm
MV=mv
CP=cp

all: modules

build: all

modules:
	$(CC) -I../include -o drone-module.so -shared -g -O3 -fPIC drone-module.c
	$(CC) -I../include -o uhm-module.so -shared -g -O3 -fPIC uhm-module.c

clean:
	$(RM) -f *.so

distclean: clean

install:
	@echo "There is no make install for the sample modules, you must copy them manually!"
