/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 ******************************************************************************/

#ifndef LUAVM_HPP
#define LUAVM_HPP

/*
This file includes all the Lua VM headers that are neede by the
Lua Vermelha JIT compiler. This header can then be included in
other parts of the JIT where access to the VM is needed. This is
done to hide some of the work needed to properly interface the
VM and the JIT.

When the `LUA_C_LINKAGE` macro is defined, all the includes are
wrapped in an `extern "C"` block to prevent name mangling when
linking the JIT with the VM compiled as C.
*/

#ifdef LUA_C_LINKAGE
extern "C" {
#endif

// Lua headers
#include "lua/lopcodes.h"
#include "lua/lobject.h"
#include "lua/lfunc.h"
#include "lua/ltm.h"
#include "lua/lstate.h"
#include "lua/ltable.h"
#include "lua/ldo.h"
#include "lua/lvm.h"
#include "lua/lgc.h"
#include "lua/ldebug.h"
#include "lua/luav.h"
#include "lua/lvjit.h"

#ifdef LUA_C_LINKAGE
}
#endif

#endif // LUAVM_HPP

