################################################################################
#
# (c) Copyright IBM Corp. 2016, 2016
#
#  This program and the accompanying materials are made available
#  under the terms of the Eclipse Public License v1.0 and
#  Apache License v2.0 which accompanies this distribution.
#
#      The Eclipse Public License is available at
#      http://www.eclipse.org/legal/epl-v10.html
#
#      The Apache License v2.0 is available at
#      http://www.opensource.org/licenses/apache2.0.php
#
# Contributors:
#    Multiple authors (IBM Corp.) - initial implementation and documentation
#
################################################################################

CXX?= g++
AR?= ar
LD?= ld

ARCH?= x
SUBARCH?= amd64

STATICLIB_NAME?= libvermelha.a
VERMELHAOBJ_NAME?=v ermelha.o
SHAREDLIB_NAME?= libvermelha.so

CXX_FLAGS= -std=c++0x -fno-rtti -W -Wall -Wextra -fPIC $(CXX_FLAGS_EXTRA) $(addprefix -I,$(IPATH))

# path definitions

BIN_DIR?= $(PWD)
STATICLIB_DEST?= $(BIN_DIR)
VERMELHAOBJ_DEST?= $(BIN_DIR)
SHAREDLIB_DEST?= $(BIN_DIR)
OBJS_DIR?= $(BIN_DIR)/objs

LUA_DIR?= ../
OMR_DIR?= ../omr

JITBUILDER_DIR?= $(OMR_DIR)/jitbuilder
JITBUILDER_ARCH_DIR?= $(JITBUILDER_DIR)/$(ARCH)
JITBUILDER_SUBARCH_DIR?= $(JITBUILDER_ARCH_DIR)/$(SUBARCH)
JITBUILDER_LIB_DIR?= $(JITBUILDER_DIR)/release
JITBUILDER_HEADERS?= $(JITBUILDER_LIB_DIR)/include

OMR_COMPILER_DIR?= $(OMR_DIR)/compiler
OMR_COMPILER_ARCH_DIR?= $(OMR_COMPILER_DIR)/$(ARCH)
OMR_COMPILER_SUBARCH_DIR?= $(OMR_COMPILER_ARCH_DIR)/$(SUBARCH)

IPATH= $(LUA_DIR) $(JITBUILDER_HEADERS) $(JITBUILDER_SUBARCH_DIR) $(JITBUILDER_ARCH_DIR) $(JITBUILDER_DIR) $(OMR_COMPILER_SUBARCH_DIR) $(OMR_COMPILER_ARCH_DIR) $(OMR_COMPILER_DIR) $(OMR_DIR)

# file list

SOURCE_FILES= \
	LuaTypeDictionary.cpp \
	LuaFunctionBuilder.cpp \
	lvermelha.cpp

OBJECT_FILES= $(addprefix $(OBJS_DIR)/, $(subst .cpp,.o,$(SOURCE_FILES)))
STATICLIB_PATH= $(STATICLIB_DEST)/$(STATICLIB_NAME)
VERMELHAOBJ_PATH= $(VERMELHAOBJ_DEST)/$(VERMELHAOBJ_NAME)
SHAREDLIB_PATH= $(SHAREDLIB_DEST)/$(SHAREDLIB_NAME)

# targets

.PHONY: default staticlib vermelhaobj sharedlib clean cleanall

default: staticlib

staticlib: $(STATICLIB_PATH)
$(STATICLIB_NAME): $(STATICLIB_PATH)
$(STATICLIB_PATH): $(OBJECT_FILES)
	$(AR) -r $@ $(OBJECT_FILES)

vermelhaobj: $(VERMELHAOBJ_PATH)
$(VERMELHAOBJ_NAME): $(VERMELHAOBJ_PATH)
$(VERMELHAOBJ_PATH): $(OBJECT_FILES)
	$(LD) -Ur -o $@ $(OBJECT_FILES) $(LD_FLAGS_EXTRA)

sharedlib: $(SHAREDLIB_PATH)
$(SHAREDLIB_NAME): $(SHAREDLIB_PATH)
$(SHAREDLIB_PATH): $(OBJECT_FILES)
	$(CXX) -shared -o $@ $(OBJECT_FILES) $(LD_FLAGS_EXTRA) $(JITBUILDER_LIB_DIR)/libjitbuilder.a -ldl

$(OBJS_DIR)/%.o: %.cpp *.hpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJECT_FILES)

cleanall:
	rm -f $(OBJECT_FILES) $(STATICLIB_PATH) $(SHAREDLIB_PATH) $(VERMELHAOBJ_PATH)
