.PHONY : all


UNAME := $(shell uname)

ifeq ($(UNAME),Darwin)
	CPP=clang++ -std=c++11 -Wall -Werror -g
endif

ifeq ($(UNAME),Linux)
	CPP=g++ -std=c++11 -Wall -Werror -g
endif


all : check get-rect

objs :
	mkdir -p objs

objs/%.o : %.cpp structures.hpp | objs
	$(CPP) -c $< -o $@

check : objs/check.o objs/structures.o
	$(CPP) $^ -o $@ -lgmp -lCGAL

get-rect : objs/get-rect.o objs/structures.o
	$(CPP) $^ -o $@ -lgmp -lCGAL
