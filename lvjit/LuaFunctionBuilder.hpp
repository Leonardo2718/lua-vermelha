/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2017
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

#ifndef LUAFUNCTIONBUILDER_HPP
#define LUAFUNCTIONBUILDER_HPP

// JitBuilder headers
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/BytecodeBuilder.hpp"

// Lua headers
#include "LuaTypeDictionary.hpp"
#include "luavm.hpp"

namespace Lua { class FunctionBuilder; }

/*
** An IL builder for Lua functions
*/
class Lua::FunctionBuilder : public TR::MethodBuilder {
public:
    FunctionBuilder(Proto* p, Lua::TypeDictionary* types);
    
    bool buildIL() override;

    /*
    All builder convenience functions take an already allocated
    builder object, generate IL for the opcode they correspond to, and
    return true if successful, false otherwise.
    */

    bool do_move(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_loadk(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_loadbool(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* dest, Instruction instruction);
    bool do_loadnil(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_gettabup(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_gettable(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_settabup(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_settable(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_newtable(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_math(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_mod(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_pow(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_div(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_idiv(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_band(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_bor(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_bxor(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_shl(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_shr(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_unm(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_bnot(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_not(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_len(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_concat(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_jmp(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_cmp(const char* cmpFunc, TR::BytecodeBuilder* builder, TR::BytecodeBuilder* dest, Instruction instruction);
    bool do_test(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* dest, Instruction instruction);
    bool do_testset(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* dest, Instruction instruction);

    bool do_call(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_return(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_forloop(TR::BytecodeBuilder* builder, TR::BytecodeBuilder* loopStart, Instruction instruction);
    bool do_forprep(TR::BytecodeBuilder* builder, Instruction instruction);

    // convenience functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /*
    These functions generate IL builders that are equivalent to the expansion of
    corresponding macros defined in the Lua VM.
    */
    void jit_setobj(TR::BytecodeBuilder* builder, TR::IlValue* obj1, TR::IlValue* obj2);
    void jit_Protect(TR::IlBuilder* builder); // updates local copies of values in case of stack reallocation
    TR::IlValue* jit_R(TR::BytecodeBuilder* builder, int arg);
    TR::IlValue* jit_K(TR::BytecodeBuilder* builder, int arg);
    TR::IlValue* jit_RK(TR::BytecodeBuilder* builder, int arg); // equivalent to RKB and RKC in `lua/lvm.c`
    TR::IlValue* jit_clLvalue(TR::IlBuilder* builder, TR::IlValue* func);
    TR::IlValue* jit_checktag(TR::IlBuilder* builder, TR::IlValue* value, TR::IlValue* type);
    TR::IlValue* jit_tonumber(TR::IlBuilder* builder, TR::IlValue* value, TR::IlValue* type);
    TR::IlValue* jit_isinteger(TR::IlBuilder* builder, TR::IlValue* type);
    TR::IlValue* jit_isnumber(TR::IlBuilder* builder, TR::IlValue* type);

private:
    Proto* prototype;
    TR::IlValue *intType;
    TR::IlValue *fltType;
    TR::IlValue *numType;
    TypeDictionary::LuaTypes luaTypes;
};

#endif // LUAFUNCTIONBUILDER_HPP
