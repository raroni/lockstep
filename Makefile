include MakefileSettings

all: server client

CC = clang++
COMMON_FLAGS = -Wall -g -std=gnu++11 -stdlib=libc++ -ferror-limit=1 -fno-exceptions -fno-rtti
COMMON_FLAGS += -DDEBUG
PRODUCT_DIR = $(BUILD_DIR)/products
OBJECTS_DIR = $(BUILD_DIR)/objects

COMMON_SOURCES = code/common/memory.cpp code/common/network.cpp code/common/serialization.cpp code/lib/assert.cpp code/common/network_messages.cpp code/common/conversion.cpp
COMMON_HEADER_INCLUDES = -iquote code

SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepServer.app
SERVER_BINARY = $(SERVER_PRODUCT_DIR)/Contents/MacOS/LockstepServer
SERVER_SOURCES = $(COMMON_SOURCES) code/server/main.cpp code/server/network.cpp code/server/client_set.cpp code/server/network_events.cpp code/server/network_commands.cpp code/lib/chunk_ring_buffer.cpp code/lib/byte_ring_buffer.cpp
SERVER_OBJS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(SERVER_SOURCES))
SERVER_DEPS = $(sort $(patsubst %, %.deps, $(SERVER_OBJS)))

CLIENT_OSX_FRAMEWORKS = CoreFoundation AppKit
CLIENT_OSX_FRAMEWORKS_FLAGS = $(addprefix -framework , $(CLIENT_OSX_FRAMEWORKS))

CLIENT_INFO_PLIST = client_info.plist
CLIENT_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepClient.app
CLIENT_CONTENTS_DIR = $(SERVER_PRODUCT_DIR)/Contents
CLIENT_BINARY = $(CLIENT_CONTENTS_DIR)/MacOS/LockstepClient
CLIENT_CPP_SOURCES = $(COMMON_SOURCES) code/client/posix_network.cpp code/lib/chunk_ring_buffer.cpp code/lib/chunk_list.cpp code/lib/byte_ring_buffer.cpp code/client/network_events.cpp code/client/network_commands.cpp code/client/client.cpp
CLIENT_OBJ_CPP_SOURCES = code/client/osx_main.mm
CLIENT_CPP_OBJS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(CLIENT_CPP_SOURCES))
CLIENT_CPP_CPP_OBJS = $(patsubst %.mm, $(OBJECTS_DIR)/%.o, $(CLIENT_OBJ_CPP_SOURCES))
CLIENT_OBJS = $(CLIENT_CPP_OBJS) $(CLIENT_CPP_CPP_OBJS)
CLIENT_DEPS = $(sort $(patsubst %, %.deps, $(CLIENT_OBJS)))

TEST_PRODUCT_DIR = $(PRODUCT_DIR)/test
TEST_BINARY = $(TEST_PRODUCT_DIR)/test
TEST_SOURCES =\
	test/main.cpp\
	test/orwell.cpp\
	test/byte_ring_buffer_test.cpp\
	test/chunk_ring_buffer_test.cpp\
	test/client_set_iterator_test.cpp\
	test/chunk_list_test.cpp\
	code/lib/assert.cpp\
	code/lib/byte_ring_buffer.cpp\
	code/lib/chunk_list.cpp\
	code/lib/chunk_ring_buffer.cpp\
	code/common/serialization.cpp\
	code/server/client_set.cpp
TEST_OBJS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(TEST_SOURCES))
TEST_DEPS = $(sort $(patsubst %, %.deps, $(TEST_OBJS)))

-include $(SERVER_DEPS)
-include $(CLIENT_DEPS)
-include $(TEST_DEPS)

$(OBJECTS_DIR)/%.o: ./%.cpp
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(COMMON_HEADER_INCLUDES) -c $< -o $@ -MMD -MF $@.deps

$(OBJECTS_DIR)/%.o: ./%.mm
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(COMMON_HEADER_INCLUDES) -fno-objc-arc -c $< -o $@ -MMD -MF $@.deps

$(SERVER_BINARY): $(SERVER_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $^ -o $@

$(CLIENT_BINARY): $(CLIENT_OBJS) $(CLIENT_INFO_PLIST)
	mkdir -p $(dir $@)
	cp -r $(CLIENT_INFO_PLIST) $(CLIENT_CONTENTS_DIR)/Info.plist
	$(CC) $(COMMON_FLAGS) $(CLIENT_OSX_FRAMEWORKS_FLAGS) $(CLIENT_OBJS) -o $@

$(TEST_BINARY): $(TEST_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $^ -o $@

server: $(SERVER_BINARY)

run_server: server
	$(SERVER_BINARY)

client: $(CLIENT_BINARY)

run_client: client
	$(CLIENT_BINARY)

test: $(TEST_BINARY)
	$(TEST_BINARY)

clean:
	rm -rf $(BUILD_DIR)
