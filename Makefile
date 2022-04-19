.PHONY: all test example

all: # Do nothing

test:
	$(MAKE) -f make_tests.mk $(TARGET)

example:
	$(MAKE) -f make_example.mk $(TARGET)
