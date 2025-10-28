# Compilador e flags
CC = gcc
CFLAGS  = -Wall -Wextra -Og
LDFLAGS =

# Nomes dos diretórios e executáveis
OBJ_DIR = obj
BIN_DIR = bin

CLI_TARGET  = http-client
CLI_SRC_DIR = client
CLI_OBJ_DIR = $(OBJ_DIR)/$(CLI_TARGET)

SRV_TARGET  = http-server
SRV_SRC_DIR = server
SRV_OBJ_DIR = $(OBJ_DIR)/$(SRV_TARGET)

# Arquivos fonte e objeto
CLI_SOURCES = $(wildcard $(CLI_SRC_DIR)/*.c)
CLI_OBJECTS = $(patsubst $(CLI_SRC_DIR)/%.c, $(CLI_OBJ_DIR)/%.o, $(CLI_SOURCES))
SRV_SOURCES = $(wildcard $(SRV_SRC_DIR)/*.c)
SRV_OBJECTS = $(patsubst $(SRV_SRC_DIR)/%.c, $(SRV_OBJ_DIR)/%.o, $(SRV_SOURCES))

# Target padrão
all: $(BIN_DIR)/$(CLI_TARGET) $(BIN_DIR)/$(SRV_TARGET)

# Target PHONY
.PHONY: all clean

# Linkar arquivos objeto e criar executáveis
$(BIN_DIR)/$(CLI_TARGET): $(CLI_OBJECTS) | $(BIN_DIR)
	$(CC) $(CLI_OBJECTS) $(LDFLAGS) -o $@

$(BIN_DIR)/$(SRV_TARGET): $(SRV_OBJECTS) | $(BIN_DIR)
	$(CC) $(SRV_OBJECTS) $(LDFLAGS) -o $@

# Compilar arquivos fonte para arquivos objeto
$(CLI_OBJ_DIR)/%.o: $(CLI_SRC_DIR)/%.c | $(CLI_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SRV_OBJ_DIR)/%.o: $(SRV_SRC_DIR)/%.c | $(SRV_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Criar diretórios se eles não existirem
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(CLI_OBJ_DIR):
	mkdir -p $(CLI_OBJ_DIR)

$(SRV_OBJ_DIR):
	mkdir -p $(SRV_OBJ_DIR)

# Limpar arquivos gerados
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
