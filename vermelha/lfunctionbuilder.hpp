#ifndef LFUNCTIONBUILDER_HPP
#define LFUNCTIONBUILDER_HPP

// JitBuilder headers
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/BytecodeBuilder.hpp"

// Lua headers
#include "ltypedictionary.hpp"
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
    builder object, build the opcode they correspond to, and
    return true if successful, false otherwise.
    */

    bool do_loadk(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_gettabup(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_settabup(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_add(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_sub(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_mul(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_idiv(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_band(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_bor(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_bxor(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_shl(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_shr(TR::BytecodeBuilder* builder, Instruction instruction);
    bool do_unm(TR::BytecodeBuilder* builder, Instruction instruction);

    bool do_return(TR::BytecodeBuilder* builder, Instruction instruction);

    // convenience functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /*
    These functions generate IL builders that are equivalent to the expansion of
    corresponding macros defined in the Lua VM.
    */
    
    void jit_setobj(TR::BytecodeBuilder* builder, TR::IlValue* obj1, TR::IlValue* obj2);
    
    TR::IlValue* jit_RK(int arg, TR::BytecodeBuilder* builder); // equivalent to RKB and RKC in `lua/lvm.c`
    
    void jit_Protect(TR::BytecodeBuilder* builder); // updates local copies of values in case of stack reallocation

private:
    Proto* prototype;
    TypeDictionary::LuaTypes luaTypes;
};

#endif // LFUNCTIONBUILDER_HPP
