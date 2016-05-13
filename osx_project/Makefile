include MakefileSettings

all: debug_server release_server client test

CC = clang++
COMMON_FLAGS = -Wall -g -std=gnu++11 -stdlib=libc++ -ferror-limit=1 -fno-exceptions -fno-rtti
debug_server: COMMON_FLAGS += -DDEBUG
PRODUCT_DIR = $(BUILD_DIR)/products
OBJ_DIR = $(BUILD_DIR)/objects

ROOT = $(realpath ./..)
CODE_ROOT = $(ROOT)/code

COMMON_SOURCES = code/lib/memory_arena.cpp code/common/posix_net.cpp code/lib/buf_view.cpp code/lib/assert.cpp code/common/net_messages.cpp code/common/conversion.cpp code/lib/chunk_list.cpp code/common/simulation.cpp code/common/order_serialization.cpp code/lib/math.cpp code/lib/seq_write.cpp
COMMON_HEADER_INCLUDES = -iquote $(CODE_ROOT)

SERVER_SOURCES = $(COMMON_SOURCES) code/server/osx_main.cpp code/server/posix_net.cpp code/server/posix_net_client_set.cpp code/server/net_events.cpp code/server/net_commands.cpp code/lib/chunk_ring_buffer.cpp code/lib/byte_ring_buffer.cpp code/server/game.cpp

DEBUG_SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/DebugLockstepServer.app
DEBUG_SERVER_BINARY = $(DEBUG_SERVER_PRODUCT_DIR)/Contents/MacOS/DebugLockstepServer
DEBUG_SERVER_OBJ_DIR = $(OBJ_DIR)/debug_server
DEBUG_SERVER_OBJS = $(patsubst %.cpp, $(DEBUG_SERVER_OBJ_DIR)/%.o, $(SERVER_SOURCES))
DEBUG_SERVER_DEPS = $(sort $(patsubst %, %.deps, $(DEBUG_SERVER_OBJS)))

RELEASE_SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepServer.app
RELEASE_SERVER_BINARY = $(RELEASE_SERVER_PRODUCT_DIR)/Contents/MacOS/LockstepServer
RELEASE_SERVER_OBJ_DIR = $(OBJ_DIR)/release_server
RELEASE_SERVER_OBJS = $(patsubst %.cpp, $(RELEASE_SERVER_OBJ_DIR)/%.o, $(SERVER_SOURCES))
RELEASE_SERVER_DEPS = $(sort $(patsubst %, %.deps, $(RELEASE_SERVER_OBJS)))

CLIENT_OSX_FRAMEWORKS = CoreFoundation AppKit OpenGL
CLIENT_OSX_FRAMEWORKS_FLAGS = $(addprefix -framework , $(CLIENT_OSX_FRAMEWORKS))

CLIENT_INFO_PLIST = client_info.plist
CLIENT_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepClient.app
CLIENT_CONTENTS_DIR = $(CLIENT_PRODUCT_DIR)/Contents
CLIENT_BINARY = $(CLIENT_CONTENTS_DIR)/MacOS/LockstepClient
CLIENT_CPP_SOURCES = $(COMMON_SOURCES) code/client/posix_net.cpp code/lib/chunk_ring_buffer.cpp code/lib/byte_ring_buffer.cpp code/client/net_events.cpp code/client/net_commands.cpp code/client/game.cpp code/client/opengl.cpp code/client/interpolation.cpp code/lib/int_seq.cpp
CLIENT_OBJ_CPP_SOURCES = code/client/osx_main.mm
CLIENT_OBJ_DIR = $(OBJ_DIR)/client
CLIENT_CPP_OBJS = $(patsubst %.cpp, $(CLIENT_OBJ_DIR)/%.o, $(CLIENT_CPP_SOURCES))
CLIENT_CPP_CPP_OBJS = $(patsubst %.mm, $(CLIENT_OBJ_DIR)/%.o, $(CLIENT_OBJ_CPP_SOURCES))
CLIENT_OBJS = $(CLIENT_CPP_OBJS) $(CLIENT_CPP_CPP_OBJS)
CLIENT_DEPS = $(sort $(patsubst %, %.deps, $(CLIENT_OBJS)))

TEST_PRODUCT_DIR = $(PRODUCT_DIR)/test
TEST_BINARY = $(TEST_PRODUCT_DIR)/test
TEST_SOURCES =\
	test/main.cpp\
	test/orwell.cpp\
	test/byte_ring_buffer_test.cpp\
	test/chunk_ring_buffer_test.cpp\
	test/posix_net_client_set_iterator_test.cpp\
	test/chunk_list_test.cpp\
	test/math_test.cpp\
	test/int_seq_test.cpp\
	code/lib/assert.cpp\
	code/lib/math.cpp\
	code/lib/int_seq.cpp\
	code/lib/byte_ring_buffer.cpp\
	code/lib/chunk_list.cpp\
	code/lib/chunk_ring_buffer.cpp\
	code/lib/buf_view.cpp\
	code/server/posix_net_client_set.cpp
TEST_OBJ_DIR = $(OBJ_DIR)/test
TEST_OBJS = $(patsubst %.cpp, $(TEST_OBJ_DIR)/%.o, $(TEST_SOURCES))
TEST_DEPS = $(sort $(patsubst %, %.deps, $(TEST_OBJS)))

-include $(DEBUG_SERVER_DEPS)
-include $(CLIENT_DEPS)
-include $(TEST_DEPS)

define CREATE_CPP_OBJ_COMMAND
mkdir -p $(dir $@)
$(CC) $(COMMON_FLAGS) $(COMMON_HEADER_INCLUDES) -c $< -o $@ -MMD -MF $@.deps
endef

$(DEBUG_SERVER_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(RELEASE_SERVER_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(TEST_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(CLIENT_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(CLIENT_OBJ_DIR)/%.o: $(ROOT)/%.mm
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(COMMON_HEADER_INCLUDES) -fno-objc-arc -c $< -o $@ -MMD -MF $@.deps

$(DEBUG_SERVER_BINARY): $(DEBUG_SERVER_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $^ -o $@

$(RELEASE_SERVER_BINARY): $(RELEASE_SERVER_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $^ -o $@

$(CLIENT_BINARY): $(CLIENT_OBJS) $(CLIENT_INFO_PLIST)
	mkdir -p $(dir $@)
	cp -r $(CLIENT_INFO_PLIST) $(CLIENT_CONTENTS_DIR)/Info.plist
	$(CC) $(COMMON_FLAGS) $(CLIENT_OSX_FRAMEWORKS_FLAGS) $(CLIENT_OBJS) -o $@

$(TEST_BINARY): $(TEST_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $^ -o $@

debug_server: $(DEBUG_SERVER_BINARY)
release_server: $(RELEASE_SERVER_BINARY)
client: $(CLIENT_BINARY)
test: $(TEST_BINARY)

clean:
	rm -rf $(BUILD_DIR)

run_debug_server: debug_server
	$(DEBUG_SERVER_BINARY) ${PLAYER_COUNT}

run_client: client
	$(CLIENT_BINARY)

run_test: test
	$(TEST_BINARY)

rs: run_debug_server
rc: run_client
rt: run_test
rp: server client
	$(SERVER_BINARY) & sleep 0.25; $(CLIENT_BINARY)
