BIN = test.exe
CPP_SRS = main.cpp argumentParser.cpp param.cpp
RM = rm -rf

$(BIN):
	g++ -std=c++20 $(CPP_SRS) -o $@

clean:
	$(RM) $(BIN)
