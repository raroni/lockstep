include MakefileSettings

all: debug_server release_server debug_client release_client test

CC = clang++
COMMON_FLAGS = -Wall -std=gnu++11 -stdlib=libc++ -ferror-limit=1 -fno-exceptions -fno-rtti
release_server release_client test: COMMON_FLAGS += -O2
debug_server debug_client test: COMMON_FLAGS += -O0 -DDEBUG -g
PRODUCT_DIR = $(BUILD_DIR)/products
OBJ_DIR = $(BUILD_DIR)/objects

ROOT = $(realpath ./..)
CODE_ROOT = $(ROOT)/code

COMMON_SOURCES = code/lib/memory_arena.cpp code/common/posix_net.cpp code/lib/buf_view.cpp code/lib/assert.cpp code/common/net_messages.cpp code/common/conversion.cpp code/lib/chunk_list.cpp code/common/simulation.cpp code/common/order_serialization.cpp code/lib/math.cpp code/lib/seq_write.cpp
COMMON_HEADER_INCLUDES = -iquote $(CODE_ROOT)

SERVER_SOURCES = $(COMMON_SOURCES) code/server/osx_main.cpp code/server/posix_net.cpp code/server/posix_net_client_set.cpp code/server/net_events.cpp code/server/net_commands.cpp code/lib/chunk_ring_buffer.cpp code/lib/byte_ring_buffer.cpp code/server/game.cpp

DEBUG_SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/debug_server
DEBUG_SERVER_BINARY = $(DEBUG_SERVER_PRODUCT_DIR)/server
DEBUG_SERVER_OBJ_DIR = $(OBJ_DIR)/debug_server
DEBUG_SERVER_OBJS = $(patsubst %.cpp, $(DEBUG_SERVER_OBJ_DIR)/%.o, $(SERVER_SOURCES))
DEBUG_SERVER_DEPS = $(sort $(patsubst %, %.deps, $(DEBUG_SERVER_OBJS)))

RELEASE_SERVER_PRODUCT_DIR = $(PRODUCT_DIR)/server
RELEASE_SERVER_BINARY = $(RELEASE_SERVER_PRODUCT_DIR)/server
RELEASE_SERVER_OBJ_DIR = $(OBJ_DIR)/release_server
RELEASE_SERVER_OBJS = $(patsubst %.cpp, $(RELEASE_SERVER_OBJ_DIR)/%.o, $(SERVER_SOURCES))
RELEASE_SERVER_DEPS = $(sort $(patsubst %, %.deps, $(RELEASE_SERVER_OBJS)))

CLIENT_OSX_FRAMEWORKS = CoreFoundation AppKit OpenGL
CLIENT_OSX_FRAMEWORKS_FLAGS = $(addprefix -framework , $(CLIENT_OSX_FRAMEWORKS))

CLIENT_CPP_SOURCES = $(COMMON_SOURCES) code/client/posix_net.cpp code/lib/chunk_ring_buffer.cpp code/lib/byte_ring_buffer.cpp code/client/net_events.cpp code/client/net_commands.cpp code/client/game.cpp code/client/opengl.cpp code/client/interpolation.cpp code/lib/int_seq.cpp
CLIENT_INFO_PLIST = client_info.plist
CLIENT_OBJ_CPP_SOURCES = code/client/osx_main.mm
CLIENT_CPP_OBJS = $(patsubst %.cpp, %.o, $(CLIENT_CPP_SOURCES))
CLIENT_OBJ_CPP_OBJS = $(patsubst %.mm, %.o, $(CLIENT_OBJ_CPP_SOURCES))
CLIENT_OBJS = $(CLIENT_CPP_OBJS) $(CLIENT_OBJ_CPP_OBJS)

DEBUG_CLIENT_PRODUCT_DIR = $(PRODUCT_DIR)/DebugLockstepClient.app
DEBUG_CLIENT_CONTENTS_DIR = $(DEBUG_CLIENT_PRODUCT_DIR)/Contents
DEBUG_CLIENT_BINARY = $(DEBUG_CLIENT_CONTENTS_DIR)/MacOS/DebugLockstepClient
DEBUG_CLIENT_OBJ_DIR = $(OBJ_DIR)/debug_client
DEBUG_CLIENT_OBJS = $(addprefix $(DEBUG_CLIENT_OBJ_DIR)/, $(CLIENT_OBJS))
DEBUG_CLIENT_DEPS = $(sort $(patsubst %, %.deps, $(DEBUG_CLIENT_OBJS)))

RELEASE_CLIENT_PRODUCT_DIR = $(PRODUCT_DIR)/LockstepClient.app
RELEASE_CLIENT_CONTENTS_DIR = $(RELEASE_CLIENT_PRODUCT_DIR)/Contents
RELEASE_CLIENT_BINARY = $(RELEASE_CLIENT_CONTENTS_DIR)/MacOS/LockstepClient
RELEASE_CLIENT_OBJ_DIR = $(OBJ_DIR)/release_client
RELEASE_CLIENT_OBJS = $(addprefix $(RELEASE_CLIENT_OBJ_DIR)/, $(CLIENT_OBJS))
RELEASE_CLIENT_DEPS = $(sort $(patsubst %, %.deps, $(RELEASE_CLIENT_OBJS)))

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
-include $(RELEASE_SERVER_DEPS)
-include $(DEBUG_CLIENT_DEPS)
-include $(RELEASE_CLIENT_DEPS)
-include $(TEST_DEPS)

define CREATE_CPP_OBJ_COMMAND
mkdir -p $(dir $@)
$(CC) $(COMMON_FLAGS) $(COMMON_HEADER_INCLUDES) -c $< -o $@ -MMD -MF $@.deps
endef

define CREATE_OBJ_CPP_OBJ_COMMAND
mkdir -p $(dir $@)
$(CC) $(COMMON_FLAGS) $(COMMON_HEADER_INCLUDES) -fno-objc-arc -c $< -o $@ -MMD -MF $@.deps
endef

define CREATE_SERVER_BINARY_COMMAND
mkdir -p $(dir $@)
$(CC) $(COMMON_FLAGS) $^ -o $@
endef

$(DEBUG_SERVER_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(RELEASE_SERVER_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(TEST_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(DEBUG_CLIENT_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(RELEASE_CLIENT_OBJ_DIR)/%.o: $(ROOT)/%.cpp
	$(CREATE_CPP_OBJ_COMMAND)

$(DEBUG_CLIENT_OBJ_DIR)/%.o: $(ROOT)/%.mm
	$(CREATE_OBJ_CPP_OBJ_COMMAND)

$(RELEASE_CLIENT_OBJ_DIR)/%.o: $(ROOT)/%.mm
	$(CREATE_OBJ_CPP_OBJ_COMMAND)

$(DEBUG_SERVER_BINARY): $(DEBUG_SERVER_OBJS)
	$(CREATE_SERVER_BINARY_COMMAND)

$(RELEASE_SERVER_BINARY): $(RELEASE_SERVER_OBJS)
	$(CREATE_SERVER_BINARY_COMMAND)

$(DEBUG_CLIENT_BINARY): $(DEBUG_CLIENT_OBJS) $(CLIENT_INFO_PLIST)
	mkdir -p $(dir $@)
	cp -r $(CLIENT_INFO_PLIST) $(DEBUG_CLIENT_CONTENTS_DIR)/Info.plist
	$(CC) $(COMMON_FLAGS) $(CLIENT_OSX_FRAMEWORKS_FLAGS) $(DEBUG_CLIENT_OBJS) -o $@

$(RELEASE_CLIENT_BINARY): $(RELEASE_CLIENT_OBJS) $(CLIENT_INFO_PLIST)
	mkdir -p $(dir $@)
	cp -r $(CLIENT_INFO_PLIST) $(RELEASE_CLIENT_CONTENTS_DIR)/Info.plist
	$(CC) $(COMMON_FLAGS) $(CLIENT_OSX_FRAMEWORKS_FLAGS) $(RELEASE_CLIENT_OBJS) -o $@

$(TEST_BINARY): $(TEST_OBJS)
	mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $^ -o $@

debug_server: $(DEBUG_SERVER_BINARY)
release_server: $(RELEASE_SERVER_BINARY)
debug_client: $(DEBUG_CLIENT_BINARY)
release_client: $(RELEASE_CLIENT_BINARY)
test: $(TEST_BINARY)

clean:
	rm -rf $(BUILD_DIR)

run_debug_server: debug_server
	$(DEBUG_SERVER_BINARY) ${PLAYER_COUNT}

run_debug_client: debug_client
	$(DEBUG_CLIENT_BINARY) ${HOST}

run_release_server: release_server
	$(RELEASE_SERVER_BINARY) ${PLAYER_COUNT}

run_release_client: release_client
	$(RELEASE_CLIENT_BINARY) ${HOST}

run_test: test
	$(TEST_BINARY)

rds: run_debug_server
rrs: run_release_server
rdc: run_debug_client
rrc: run_release_client
rt: run_test
rdp: debug_server debug_client
	$(DEBUG_SERVER_BINARY) & sleep 0.25; $(DEBUG_CLIENT_BINARY)
rrp: release_server release_client
	$(RELEASE_SERVER_BINARY) & sleep 0.25; $(RELEASE_CLIENT_BINARY)
