CC=clang
CFLAGS=-std=c++11 -I/usr/include/x86_64-linux-gnu/c++/4.8/ -I../include/ -Wall
LDFLAGS=-lstdc++
OLVL=-O2

# NOTE: This Makefile has only been used on the author's personal system, you
# may well have to tweak the above parameters to get it to work on your own
# platform!

# For instance, if you want to build with gcc instead, the above should be
# changed to:
# CC=g++
# CFLAGS=-std=c++11 -I../include/ -Wall
# LDFLAGS=

all: release

release: main.cpp either_tests.h maybe_tests.h base.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(OLVL) -o ftl_tests main.cpp

debug: main.cpp either_tests.h maybe_tests.h base.h
	$(CC) $(CFLAGS) $(LDFLAGS) -g -o ftl_tests main.cpp

