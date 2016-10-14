ARCH?=x
SUBARCH?=amd64

CXX?= g++
AR?= ar
LD?= ld

CXX_FLAGS= -std=c++0x -fno-rtti -W -Wall -Wextra -fPIC $(addprefix -I,$(IPATH)) $(CXX_FLAGS_EXTRA)

VERMELHAOBJ_NAME?=vermelha.o
STATICLIB_NAME?=libvermelha.a
SHAREDLIB_NAME?=libvermelha.so

# path definitions

BIN_DIR?=$(PWD)
VERMELHAOBJ_DEST?=$(BIN_DIR)
STATICLIB_DEST?=$(BIN_DIR)
SHAREDLIB_DEST?=$(BIN_DIR)
OBJS_DIR?=$(BIN_DIR)/objs

LUA_DIR?=../
OMR_DIR?=../omr

JITBUILDER_DIR?=$(OMR_DIR)/jitbuilder
JITBUILDER_ARCH_DIR?=$(JITBUILDER_DIR)/$(ARCH)
JITBUILDER_SUBARCH_DIR?=$(JITBUILDER_ARCH_DIR)/$(SUBARCH)
JITBUILDER_RELEASE_DIR?=$(JITBUILDER_DIR)/release/include

OMR_COMPILER_DIR?=$(OMR_DIR)/compiler
OMR_COMPILER_ARCH_DIR?=$(OMR_COMPILER_DIR)/$(ARCH)
OMR_COMPILER_SUBARCH_DIR?=$(OMR_COMPILER_ARCH_DIR)/$(SUBARCH)

IPATH= $(LUA_DIR) $(JITBUILDER_RELEASE_DIR) $(JITBUILDER_SUBARCH_DIR) $(JITBUILDER_ARCH_DIR) $(JITBUILDER_DIR) $(OMR_COMPILER_SUBARCH_DIR) $(OMR_COMPILER_ARCH_DIR) $(OMR_COMPILER_DIR) $(OMR_DIR) $(IPATH_EXTRA)

# file list

SOURCE_FILES= \
	ltypedictionary.cpp \
	lfunctionbuilder.cpp \
	lvermelha.cpp

OBJECT_FILES= $(addprefix $(OBJS_DIR)/, $(subst .cpp,.o,$(SOURCE_FILES)))
VERMELHAOBJ_PATH=$(VERMELHAOBJ_DEST)/$(VERMELHAOBJ_NAME)
STATICLIB_PATH=$(STATICLIB_DEST)/$(STATICLIB_NAME)
SHAREDLIB_PATH=$(SHAREDLIB_DEST)/$(SHAREDLIB_NAME)

# targets

.PHONY: all staticlib sharedlib vermelhaobj clean

all: staticlib sharedlib vermelhaobj

staticlib: $(STATICLIB_PATH)

sharedlib: $(SHAREDLIB_PATH)

vermelhaobj: $(VERMELHAOBJ_PATH)

$(STATICLIB_PATH): $(OBJECT_FILES)
	$(AR) -r $@ $(OBJECT_FILES)

$(SHAREDLIB_PATH): $(OBJECT_FILES)
	$(CXX) -shared $(CXX_FLAGS) -o $@ $(OBJECT_FILES) ../omr/jitbuilder/release/libjitbuilder.a -ldl

$(VERMELHAOBJ_PATH): $(OBJECT_FILES)
	$(LD) -Ur -o $@ $(OBJECT_FILES) ../omr/jitbuilder/release/libjitbuilder.a -L/lib -L/usr/lib -lstdc++

$(OBJS_DIR)/%.o: %.cpp *.hpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@ -L../omr/jitbuilder/release -ljitbuilder -ldl

clean:
	rm -f $(OBJECT_FILES)

cleanall:
	rm -f $(OBJECT_FILES) $(STATICLIB_PATH) $(SHAREDLIB_PATH) $(VERMELHAOBJ_PATH)
