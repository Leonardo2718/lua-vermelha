#ifndef LFUNCTIONBUILDER_HPP
#define LFUNCTIONBUILDER_HPP

// JitBuilder headers
#include "ilgen/MethodBuilder.hpp"

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

private:
    Proto* prototype;
    TypeDictionary::LuaTypes luaTypes;
};

#endif // LFUNCTIONBUILDER_HPP
