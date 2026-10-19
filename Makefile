CC 		:= gcc
AR		:= ar
CFLAGS 	:= -std=c11 -Wall -Wextra -Wpedantic -Wconversion -O2 -g -Iinclude

BUILD_DIR 	:= build
LIB_DIR 	:= src
TEST_DIR	:= tests

LIB_FILE	:= $(wildcard src/*.c) 
TEST_FILE	:= $(wildcard tests/*.c) 

LIB_OBJ		:= $(LIB_FILE:src/%.c=build/%.o)
TEST_OBJ	:= $(TEST_FILE:tests/%.c=build/tests/%)

DEP := $(LIB_OBJ:.o=.d)
PREFIX  := /usr/local

.PHONY: all test clean

all: $(LIB_OBJ)
	@$(AR) rcs $(BUILD_DIR)/ds_lib.a $^

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/tests: $(BUILD_DIR)
	mkdir -p $@

# Compilazione di ogni .c in .o
$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP

# Compila ogni test 
$(BUILD_DIR)/tests/%: $(TEST_DIR)/%.c $(LIB_OBJ) | $(BUILD_DIR)/tests
	$(CC) $(CFLAGS) $^ -o $@


clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned!"

# Azione per eseguire i test
test: $(TEST_OBJ)
	@for t in $(TEST_OBJ); do echo "== $$t =="; ./$$t || exit 1; done;

install: $(LIB_OBJ)
	install -d $(PREFIX)/include $(PREFIX)/lib
	cp -r include/ds_lib $(PREFIX)/include/
	install -m 644 $(BUILD_DIR)/ds_lib.a $(PREFIX)/lib/

-include $(DEP)


