BIN = example_
CXX = g++
CXXFLAGS = -std=c++17 -g

CPP_SRC = example/example.cpp $(wildcard src/*.cpp)
INCLUDES = includes/

include include_make.mk
