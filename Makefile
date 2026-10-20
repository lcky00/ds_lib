CC 		:= gcc
AR		:= ar
CFLAGS 	:= -std=c11 -Wall -Wextra -Wpedantic -Wconversion -O2 -g -Iinclude

BUILD_DIR 	:= build
LIB_DIR 	:= src
TEST_DIR	:= tests

PREFIX  := /usr/local

LIB_NAME := libds_lib.a
LIB := $(BUILD_DIR)/$(LIB_NAME)

LIB_FILE	:= $(wildcard src/*.c) 
TEST_FILE	:= $(wildcard tests/*.c) 

LIB_OBJ		:= $(LIB_FILE:$(LIB_DIR)/%.c=$(BUILD_DIR)/%.o)
TEST_OBJ	:= $(TEST_FILE:$(TEST_DIR)/%.c=$(BUILD_DIR)/tests/%)

DEP := $(LIB_OBJ:.o=.d)


.PHONY: all test clean install

all: $(LIB)

$(LIB): $(LIB_OBJ)
	@$(AR) rcs $@ $^

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/tests: $(BUILD_DIR)
	mkdir -p $@

# Compilazione di ogni .c in .o
$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP

# Compila ogni test 
$(BUILD_DIR)/tests/%: $(TEST_DIR)/%.c $(LIB) | $(BUILD_DIR)/tests
	$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned!"

# Azione per eseguire i test
test: $(TEST_OBJ)
	@for t in $(TEST_OBJ); do echo "== $$t =="; ./$$t || exit 1; done;

install:
	@test -f $(LIB) || { echo "$(LIB) non trovato: esegui prima 'make'"; exit 1; }
	install -d $(PREFIX)/include/ds_lib $(PREFIX)/lib
	install -m 644 include/ds_lib/*.h $(PREFIX)/include/ds_lib/
	install -m 644 $(LIB) $(PREFIX)/lib/

-include $(DEP)


