CC      ?= cc
AR      ?= ar
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -g
CPPFLAGS += -Iinclude
PREFIX  ?= /usr/local

# make SAN=1 -> compila con AddressSanitizer e UBSan
ifdef SAN
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif

BUILD := build
LIB   := $(BUILD)/libds_lib.a

SRC   := $(wildcard src/*.c)
OBJ   := $(SRC:src/%.c=$(BUILD)/%.o)
DEP   := $(OBJ:.o=.d)

TEST_SRC := $(wildcard tests/test_*.c)
TEST_BIN := $(TEST_SRC:tests/%.c=$(BUILD)/%)

.PHONY: all test asan install clean

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/test_%: tests/test_%.c $(LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LIB) -o $@

$(BUILD):
	mkdir -p $@

# Compila ed esegue tutti i tests/test_*.c
test: $(TEST_BIN)
	@for t in $(TEST_BIN); do \
		echo "== $$t"; \
		./$$t || exit 1; \
	done

# Ricompila da zero con i sanitizer e lancia i test
asan: clean
	$(MAKE) test SAN=1

install: $(LIB)
	install -d $(PREFIX)/include $(PREFIX)/lib
	cp -r include/ds_lib $(PREFIX)/include/
	install -m 644 $(LIB) $(PREFIX)/lib/

clean:
	rm -rf $(BUILD)

-include $(DEP)
