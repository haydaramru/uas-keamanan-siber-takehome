CC = gcc
CFLAGS = -Wall -Iinclude

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

SOURCES = $(wildcard $(SRC_DIR)/*.c)

OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

EXECUTABLES = $(BIN_DIR)/kdc $(BIN_DIR)/alice $(BIN_DIR)/bob

.PHONY: all
all: $(EXECUTABLES)

$(BIN_DIR) $(OBJ_DIR):
	@echo "Creating directory $@..."
	@mkdir -p $@

$(BIN_DIR)/kdc: $(OBJ_DIR)/kdc.o | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/alice: $(OBJ_DIR)/alice.o | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/bob: $(OBJ_DIR)/bob.o | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(CFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $< -> $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	@echo "Cleaning up generated files..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: run
run: all
	@echo "Starting servers... (Press Ctrl+C to stop)"
	@trap 'kill 0' SIGINT; \
	$(BIN_DIR)/kdc & \
	$(BIN_DIR)/bob & \
	sleep 2 && \
	$(BIN_DIR)/alice && \
	echo "Alice finished. Shutting down servers." && \
	killall kdc bob