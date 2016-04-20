include MakefileSettings

all: server client

CC = clang++
COMMON_FLAGS = -Wall -g -std=gnu++11 -stdlib=libc++ -ferror-limit=1 -fno-exceptions -fno-rtti
COMMON_FLAGS += -DDEBUG
PRODUCT_DIR = $(BUILD_DIR)/products
OBJECTS_DIR = $(BUILD_DIR)/objects

CODE_ROOT = ../code

COMMON_SOURCES = $(CODE_ROOT)/common/memory.cpp $(CODE_ROOT)/common/network.cpp $(CODE_ROOT)/common/serialization.cpp $(CODE_ROOT)/lib/assert.cpp $(CODE_ROOT)/common/network_messages.cpp $(CODE_ROOT)/common/conversion.cpp
COMMON_HEADER_INCLUDES = -iquote $(CODE_ROOT)

SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepServer.app
SERVER_BINARY = $(SERVER_PRODUCT_DIR)/Contents/MacOS/LockstepServer
SERVER_SOURCES = $(COMMON_SOURCES) $(CODE_ROOT)/server/main.cpp $(CODE_ROOT)/server/network.cpp $(CODE_ROOT)/server/client_set.cpp $(CODE_ROOT)/server/network_events.cpp $(CODE_ROOT)/server/network_commands.cpp $(CODE_ROOT)/lib/chunk_ring_buffer.cpp $(CODE_ROOT)/lib/byte_ring_buffer.cpp
SERVER_OBJS = $(patsubst %.cpp, $(OBJECTS_DIR)/%.o, $(SERVER_SOURCES))
SERVER_DEPS = $(sort $(patsubst %, %.deps, $(SERVER_OBJS)))

CLIENT_OSX_FRAMEWORKS = CoreFoundation AppKit OpenGL
CLIENT_OSX_FRAMEWORKS_FLAGS = $(addprefix -framework , $(CLIENT_OSX_FRAMEWORKS))

CLIENT_INFO_PLIST = client_info.plist
CLIENT_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepClient.app
CLIENT_CONTENTS_DIR = $(SERVER_PRODUCT_DIR)/Contents
CLIENT_BINARY = $(CLIENT_CONTENTS_DIR)/MacOS/LockstepClient
CLIENT_CPP_SOURCES = $(COMMON_SOURCES) $(CODE_ROOT)/client/posix_network.cpp $(CODE_ROOT)/lib/chunk_ring_buffer.cpp $(CODE_ROOT)/lib/chunk_list.cpp $(CODE_ROOT)/lib/byte_ring_buffer.cpp $(CODE_ROOT)/client/network_events.cpp $(CODE_ROOT)/client/network_commands.cpp $(CODE_ROOT)/client/client.cpp $(CODE_ROOT)/client/opengl.cpp
CLIENT_OBJ_CPP_SOURCES = $(CODE_ROOT)/client/osx_main.mm
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
	$(CODE_ROOT)/lib/assert.cpp\
	$(CODE_ROOT)/lib/byte_ring_buffer.cpp\
	$(CODE_ROOT)/lib/chunk_list.cpp\
	$(CODE_ROOT)/lib/chunk_ring_buffer.cpp\
	$(CODE_ROOT)/common/serialization.cpp\
	$(CODE_ROOT)/server/client_set.cpp
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
