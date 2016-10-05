LUA_DIR?= $(PWD)/lua
VERMELHA_DIR?= $(PWD)/vermelha

VERMELHA_NAME?= vermelha
VERMELHA_LIB?= lib$(VERMELHA_NAME).a
VERMELHA_PATH?= $(VERMELHA_DIR)/$(VERMELHA_LIB)

.PHONY: all lua luac $(VERMELHA_LIB)

all: lua $(VERMELHA_PATH)

lua luac: $(VERMELHA_PATH)
#	cd $(LUA_DIR) && $(MAKE) PLAT=linux CC="g++" MYCFLAGS="-fpermissive" MYLDFLAGS="-L$(VERMELHA_DIR)" MYLIBS="-l$(VERMELHA_NAME)"
	cd $(LUA_DIR) && $(MAKE) PLAT=linux CC="g++" MYCFLAGS="-fpermissive" MYLDFLAGS="-L$(VERMELHA_DIR) -L$(PWD)/omr/jitbuilder/release" MYLIBS="-l$(VERMELHA_NAME) -ljitbuilder"
#	cd $(LUA_DIR) && $(MAKE) PLAT=linux CC="g++" MYCFLAGS="-fpermissive" MYOBJS="$(VERMELHA_PATH)" MYLDFLAGS="-L/lib -L/usr/lib" MYLIBS="-lstdc++"

$(VERMELHA_PATH):
	cd $(VERMELHA_DIR) && $(MAKE) $@ CXX_FLAGS_EXTRA="-fpermissive"

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

