# Thinks to define before including:
# CFLAGS
# CXXFLAGS
# BIN		-> Binary name to be created
# INCLUDES	-> directories with include files
# C_SRC		-> All c source files to compile
# CPP_SRC	-> All cpp source files to compile
# LDFLAGS 	-> Names of libraries to use
# LDPATH 	-> Paths to all the libraries

MV := mv -f
RM := rm -rf

CC = gcc
CXX = g++

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

AR = ar
ARFLAGS = rs

OBJDIR := obj
DEPDIR := $(OBJDIR)/.deps

OBJECTS := $(C_SRC:%.c=$(OBJDIR)/%.o) $(CPP_SRC:%.cpp=$(OBJDIR)/%.o)
DEPFILES := $(CPP_SRC:%.cpp=$(DEPDIR)/%.d) $(C_SRC:%.c=$(DEPDIR)/%.d)
INC_PAT = $(addprefix -I, $(INCLUDES))

# GCC will not create this directiories automaticly
$(shell mkdir -p $(dir $(DEPFILES)) >/dev/null)
$(shell mkdir -p $(dir $(OBJECTS)) >/dev/null)


CXXFLAGS += $(INC_PAT)

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c

COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c


# Compile C files into object files
$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d | $(DEPDIR)
	@echo "Compiling: $<"
	@$(COMPILE.c) $(OUTPUT_OPTION) $<

# Compile C++ files into object files
$(OBJDIR)/%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	@echo "Compiling: $<"
	@$(COMPILE.cc) $(OUTPUT_OPTION) $<

.PHONY: all
all: $(library) $(BIN)

$(BIN): $(OBJECTS)
	@echo "Linking to executable: $@"
	@$(CXX) $(OBJECTS) $(LDPATH) $(LDFLAGS) -o $@

.PHONY: clean
clean: 
	$(RM) $(OBJDIR) $(BIN) $(extra_clean)

$(DEPFILES):

include $(wildcard $(DEPFILES))

# For automatic dependencies generation see:
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/#placeobj
