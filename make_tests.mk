BIN = test
CXX = g++
CXXFLAGS = -std=c++17 -g
LDFLAGS = -lgtest -lgtest_main

CPP_SRC = tests/funstional_tests.cpp $(wildcard src/*.cpp)
INCLUDES = includes/

.PHONY: run
run: all
	@echo -e "\nExecuting tests: \n"
	@./test.exe

include include_make.mk
