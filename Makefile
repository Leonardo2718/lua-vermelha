CC?= gcc
CXX?= g++

LUA?= lua
LUAC?= luac
VERMELHA?= vermelha

LUA_LIB?= liblua.a
VERMELHA_LIB?= lib$(VERMELHA).a
JITBUILDER_LIB?= libjitbuilder.a

LUA_DIR?= $(PWD)/lua
VERMELHA_DIR?= $(PWD)/vermelha
JITBUILDER_DIR?= $(PWD)/omr/jitbuilder

LUA_PATH?= $(LUA_DIR)/$(LUA)
VERMELHA_PATH?= $(VERMELHA_DIR)/$(VERMELHA_LIB)
JITBUILDER_PATH?= $(JITBUILDER_DIR)/$(JITBUILDER_LIB)


.PHONY: all lua luac $(VERMELHA) $(VERMELHA_LIB) $(VERMELHA_PATH)

all: $(LUA) $(LUAC) $(VERMELHA)

$(LUAC): $(LUA)
$(LUA): $(VERMELHA_LIB)
#	cd $(LUA_DIR) && $(MAKE) linux CC="g++" MYCFLAGS="-fpermissive -g" MYLDFLAGS="-L$(VERMELHA_DIR)" MYLIBS="-l$(VERMELHA)"
	cd $(LUA_DIR) && $(MAKE) linux CC="$(CXX)" MYCFLAGS="-fpermissive -g" MYLDFLAGS="-L$(VERMELHA_DIR) -L$(PWD)/omr/jitbuilder/release" MYLIBS="-l$(VERMELHA) -ljitbuilder -ldl"
#	cd $(LUA_DIR) && $(MAKE) linux CC="g++" MYCFLAGS="-fpermissive -g" MYOBJS="$(VERMELHA_PATH)" MYLDFLAGS="-L/lib -L/usr/lib" MYLIBS="-lstdc++"

#$(LUAC): $(VERMELHA_LIB)
#	cd $(LUA_DIR) && $(MAKE) linux CC="g++" MYCFLAGS="-fpermissive -g" MYLDFLAGS="-L$(VERMELHA_DIR)" MYLIBS="-l$(VERMELHA)"
#	cd $(LUA_DIR) && $(MAKE) linux CC="g++" MYCFLAGS="-fpermissive -g" MYLDFLAGS="-L$(VERMELHA_DIR) -L$(PWD)/omr/jitbuilder/release" MYLIBS="-l$(VERMELHA) -ljitbuilder -ldl"
#	cd $(LUA_DIR) && $(MAKE) linux CC="g++" MYCFLAGS="-fpermissive -g" MYOBJS="$(VERMELHA_PATH)" MYLDFLAGS="-L/lib -L/usr/lib" MYLIBS="-lstdc++"

$(VERMELHA): $(VERMELHA_PATH)
$(VERMELHA_LIB): $(VERMELHA_PATH)
$(VERMELHA_PATH):
	cd $(VERMELHA_DIR) && $(MAKE) $@ CXX="$(CXX)" CXX_FLAGS_EXTRA="-fpermissive -g"

$(JITBUILDER_LIB):
	cd $(JITBUILDER_DIR) && $(MAKE) CXX="$(CXX)" CXX_FLAGS_EXTRA="-g"

clean:
	cd $(LUA_DIR) && $(MAKE) clean
	cd $(VERMELHA_DIR) && $(MAKE) clean

cleanlua:
	cd $(LUA_DIR) && $(MAKE) clean

cleanvermelha:
	cd $(VERMELHA_DIR) && $(MAKE) cleanall

cleanall:
	cd $(LUA_DIR) && $(MAKE) clean
	cd $(VERMELHA_DIR) && $(MAKE) cleanall

# lua patch 

# version of lua to diff against
LUA?=luavm

# commit with the applied changes
COMMIT?=HEAD

PATCH_DEST?=$(PWD)
PATCH_FILE?=$(LUA)-$(COMMIT).patch

lua-patch:
	git diff $(LUA) $(COMMIT) -- lua/* > $(PATCH_DEST)/$(PATCH_FILE)

