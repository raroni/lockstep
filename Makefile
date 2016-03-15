include MakefileSettings

all: server client

CC = clang++
CPP_FLAGS = -Wall -g -std=gnu++11 -stdlib=libc++ -ferror-limit=1 -fno-exceptions -fno-rtti
PRODUCT_DIR = $(BUILD_DIR)/products
OBJECTS_DIR = $(BUILD_DIR)/objects

SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepServer.app
SERVER_BINARY = $(SERVER_PRODUCT_DIR)/Contents/MacOS/LockstepServer
SERVER_SOURCES = code/server/main.cpp
SERVER_OBJS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(SERVER_SOURCES))
SERVER_DEPS = $(sort $(patsubst %, %.deps, $(SERVER_OBJS)))

CLIENT_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepClient.app
CLIENT_BINARY = $(SERVER_PRODUCT_DIR)/Contents/MacOS/LockstepClient
CLIENT_SOURCES = code/client/main.cpp
CLIENT_OBJS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(CLIENT_SOURCES))
CLIENT_DEPS = $(sort $(patsubst %, %.deps, $(CLIENT_OBJS)))

-include $(CLIENT_DEPS)
-include $(SERVER_DEPS)

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
