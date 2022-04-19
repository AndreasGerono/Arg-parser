BIN = example.exe
CXX = g++
CXXFLAGS = -std=c++17 -g

CPP_SRC = example.cpp src/argumentParser.cpp src/param.cpp
INCLUDES = includes/

include include_make.mk
