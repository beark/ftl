CC=g++
CFLAGS=-std=c++11 -I../include/ -Wall -Wextra
LDFLAGS=-pthread
OLVL=-O2

# NOTE: This Makefile has only been used on the author's personal system, you
# may well have to tweak the above parameters to get it to work on your own
# platform, or if you wish to use another compiler.

all: release

release: main.cpp base.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(OLVL) -o ftl_tests main.cpp

debug: main.cpp base.h
	$(CC) $(CFLAGS) $(LDFLAGS) -g -o ftl_tests main.cpp

# Target provided for reference, may well not work on your system. The async/future tests crashes under clang in linux.
clang: main.cpp base.h
	clang -std=c++11 -lstdc++ -pthread -I/usr/include/x86_64-linux-gnu/c++/4.8/ -I../include/ -Wall -o ftl_tests main.cpp

