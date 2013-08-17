CPP=g++
CXXFLAGS=-std=c++11 -I../include/ -Wall -Wextra
LDFLAGS=-pthread
OLVL=-O2

TEST_OBJS=concept_tests.o either_tests.o eithert_tests.o functional_tests.o\
	  future_tests.o fwdlist_tests.o lazy_tests.o lazyt_tests.o\
	  list_tests.o map_tests.o maybe_tests.o maybet_tests.o memory_tests.o\
	  ord_tests.o prelude_tests.o set_tests.o string_tests.o tuple_tests.o\
	  unordered_map_tests.o vector_tests.o

# NOTE: This Makefile has only been used on the author's personal system, you
# may well have to tweak the above parameters to get it to work on your own
# platform, or if you wish to use another compiler.

.PHONY: all clean

all: ftl_tests
	
ftl_tests: main.o $(TEST_OBJS)
	$(CPP) $(LDFLAGS) $(OLVL) -o ftl_tests main.o $(TEST_OBJS)

main.o: main.cpp base.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o  main.o main.cpp

concept_tests.o: concept_tests.cpp concept_tests.h base.h ../include/ftl/maybe.h ../include/ftl/vector.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o concept_tests.o concept_tests.cpp

either_tests.o: either_tests.cpp either_tests.h base.h ../include/ftl/either.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o either_tests.o either_tests.cpp

eithert_tests.o: eithert_tests.cpp eithert_tests.h base.h ../include/ftl/either_trans.h ../include/ftl/list.h ../include/ftl/functional.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o eithert_tests.o eithert_tests.cpp

functional_tests.o: functional_tests.cpp functional_tests.h base.h ../include/ftl/functional.h ../include/ftl/ord.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o functional_tests.o functional_tests.cpp

future_tests.o: future_tests.cpp future_tests.h base.h ../include/ftl/future.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o future_tests.o future_tests.cpp

fwdlist_tests.o: fwdlist_tests.cpp fwdlist_tests.h base.h ../include/ftl/forward_list.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o fwdlist_tests.o fwdlist_tests.cpp

lazy_tests.o: lazy_tests.cpp lazy_tests.h base.h ../include/ftl/lazy.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o lazy_tests.o lazy_tests.cpp

lazyt_tests.o: lazyt_tests.cpp lazyt_tests.h base.h ../include/ftl/lazy_trans.h ../include/ftl/functional.h ../include/ftl/maybe.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o lazyt_tests.o lazyt_tests.cpp

list_tests.o: list_tests.cpp list_tests.h base.h ../include/ftl/list.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o list_tests.o list_tests.cpp

map_tests.o: map_tests.cpp map_tests.h base.h ../include/ftl/map.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o map_tests.o map_tests.cpp

maybe_tests.o: maybe_tests.cpp maybe_tests.h base.h ../include/ftl/maybe.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o maybe_tests.o maybe_tests.cpp

maybet_tests.o: maybet_tests.cpp maybet_tests.h base.h ../include/ftl/maybe_trans.h ../include/ftl/vector.h ../include/ftl/functional.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o maybet_tests.o maybet_tests.cpp

memory_tests.o: memory_tests.cpp memory_tests.h base.h ../include/ftl/memory.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o memory_tests.o memory_tests.cpp

ord_tests.o: ord_tests.cpp ord_tests.h base.h ../include/ftl/ord.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o ord_tests.o ord_tests.cpp

prelude_tests.o: prelude_tests.cpp prelude_tests.h base.h ../include/ftl/prelude.h ../include/ftl/maybe.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o prelude_tests.o prelude_tests.cpp

set_tests.o: set_tests.cpp set_tests.h base.h ../include/ftl/set.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o set_tests.o set_tests.cpp

string_tests.o: string_tests.cpp string_tests.h base.h ../include/ftl/string.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o string_tests.o string_tests.cpp

tuple_tests.o: tuple_tests.cpp tuple_tests.h base.h ../include/ftl/tuple.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o tuple_tests.o tuple_tests.cpp

unordered_map_tests.o: unordered_map_tests.cpp unordered_map_tests.h base.h ../include/ftl/unordered_map.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o unordered_map_tests.o unordered_map_tests.cpp

vector_tests.o: vector_tests.cpp vector_tests.h base.h ../include/ftl/vector.h
	$(CPP) -c $(CXXFLAGS) $(OLVL) -o vector_tests.o vector_tests.cpp

clean: 
	rm -f *.o ftl_tests

