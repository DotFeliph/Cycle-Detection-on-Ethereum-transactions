CC     := gcc
CFLAGS := -Wall -g -O3 -march=native -funroll-loops -Isrc
LDLIBS := -lgmp

SRC_DIR   := src
BUILD_DIR := build

SRC_NAMES := main.c cli_parser.c graph.c wei_parser.c
SRCS      := $(addprefix $(SRC_DIR)/, $(SRC_NAMES))
OBJS      := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_NAMES))
BIN       := main

.
.PHONY: all clean docs


all: $(BIN)

$(BIN): $(OBJS)
	@echo "LD   $@"
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "CC   $<"
	$(CC) $(CFLAGS) -c $< -o $@


$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/wei_parser.o: $(SRC_DIR)/wei_parser.h
$(BUILD_DIR)/cli_parser.o: $(SRC_DIR)/cli_parser.h
$(BUILD_DIR)/graph.o:      $(SRC_DIR)/graph.h $(SRC_DIR)/uthash.h $(SRC_DIR)/wei_parser.h
$(BUILD_DIR)/main.o:       $(SRC_DIR)/cli_parser.h $(SRC_DIR)/graph.h

clean:
	@echo "CLEAN"
	@rm -rf $(BUILD_DIR) $(BIN) docs


docs:
	@echo "Generating Doxygen documentation..."
	@doxygen Doxyfile
