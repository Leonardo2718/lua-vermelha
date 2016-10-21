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

    bool do_return(TR::BytecodeBuilder* builder, Instruction instruction);

private:
    Proto* prototype;
    TypeDictionary::LuaTypes luaTypes;
};

#endif // LFUNCTIONBUILDER_HPP
