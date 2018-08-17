CFLAGS ?= -O0 -g -Wall -Werror   

CC ?= cc
ifeq "$(CC)" "cc"
	ifneq "$(shell which cc)" ""
		CC := $(shell which cc)
	endif
	ifneq "$(shell which gcc)" ""
		CC := $(shell which gcc)
	endif
	ifneq "$(shell which clang)" ""
		CC := $(shell which clang)
	endif
endif

all: print_info cast

default: all

print_info:
	@echo "Using compiler: $(CC)"

cast: $(shell ls src/*)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

.PHONY: print_info

