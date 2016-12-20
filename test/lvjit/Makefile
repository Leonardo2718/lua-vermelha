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

CXX?=g++

LUA_DIR?=../../lua
LVJIT_DIR?=../../lvjit
JITBUILDER_DIR?=../../omr/jitbuilder/release

test: compile_test initcounter_test jitflags_test
	./compile_test
	./initcounter_test
	./jitflags_test

%_test: %_test.cpp
	$(CXX) $< -I$(LUA_DIR) -L$(LUA_DIR) -L$(LVJIT_DIR) -L$(JITBUILDER_DIR) -llua -llvjit -ljitbuilder -ldl -o $@

clean:
	rm -f *_test
