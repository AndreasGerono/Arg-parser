BIN = test.exe
CPP_SRS = main.cpp
RM = rm - rf

$(BIN):
	g++ $(CPP_SRS) -o $@

clear:
	$(RM) $(BIN)
