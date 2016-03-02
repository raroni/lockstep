include MakefileSettings

# TODO: Setup deps!

all: server client

CC = clang++
CPP_FLAGS = -Wall -g -std=gnu++11 -stdlib=libc++ -ferror-limit=1 -fno-exceptions -fno-rtti
PRODUCT_DIR = $(BUILD_DIR)/products
OBJECTS_DIR = $(BUILD_DIR)/objects
SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepServer.app
SERVER_BINARY = $(SERVER_PRODUCT_DIR)/Contents/MacOS/LockstepServer
SERVER_OBJS = code/server/main.cpp
CLIENT_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepClient.app
CLIENT_BINARY = $(SERVER_PRODUCT_DIR)/Contents/MacOS/LockstepClient
CLIENT_OBJS = code/client/main.cpp


$(OBJECTS_DIR)/%.o: ./%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CPP_FLAGS) $(COMMON_HEADER_INCLUDES) -c $< -o $@ -MMD -MF $@.deps

$(SERVER_BINARY): $(SERVER_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(CPP_FLAGS) $^ -o $@

$(CLIENT_BINARY): $(CLIENT_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(CPP_FLAGS) $^ -o $@

server: $(SERVER_BINARY)

run_server: server
	$(SERVER_BINARY)

client: $(CLIENT_BINARY)

run_client: client
	$(CLIENT_BINARY)

clean:
	rm -rf $(BUILD_DIR)
