// vermelha headers
#include "lfunctionbuilder.hpp"
#include <stdio.h>

// JitBuilder headers
#include "ilgen/BytecodeBuilder.hpp"

static void printAddr(unsigned char* addr) {
   printf("Compiled! (%p)\n", addr);
}

Lua::FunctionBuilder::FunctionBuilder(Proto* p, Lua::TypeDictionary* types)
   : TR::MethodBuilder(types), prototype(p), luaTypes(types->getLuaTypes()) {

   char file_name_buff[1024];
   snprintf(file_name_buff, 1024, "%s", getstr(prototype->source));

   char line_num_buff[1024];
   snprintf(line_num_buff, 1024, "%d", prototype->linedefined);

   // since Lua functions do not have actual names, use the format
   // `<filename>_<first line of definition>_<last line of definition>`
   // as the "unique" function name
   char func_name_buff[1024];
   snprintf(func_name_buff, 1024, "%s_%s_%d", file_name_buff, line_num_buff, prototype->lastlinedefined);

   DefineLine(line_num_buff);
   DefineFile(file_name_buff);

   DefineName(func_name_buff);
   DefineParameter("L", types->PointerTo("lua_State"));
   DefineReturnType(NoType);
   //setUseBytecodeBuilders();

   DefineFunction((char*)"luaD_poscall", (char*)"ldo.c", (char*)"453",
                  (void*)luaD_poscall, Int32,
                  4,
                  types->PointerTo(luaTypes.lua_State),
                  types->PointerTo(luaTypes.CallInfo),
                  luaTypes.StkId,
                  Int32);
   DefineFunction((char*)"printAddr", (char*)"0", (char*)"0", (void*)printAddr, NoType, 1, Address);
}

bool Lua::FunctionBuilder::buildIL() {
   auto instructionCount = prototype->sizecode;
   auto instructions = prototype->code;

   auto L = Load("L");
   auto ci = LoadIndirect("lua_State", "ci", L);
   auto base = LoadIndirect("CallInfo", "u_l_base", ci);

   for (auto i = instructions; i - instructions < instructionCount; ++i) {
      auto arg_a = GETARG_A(*i);
      auto ra = IndexAt(luaTypes.StkId, base, ConstInt32(arg_a));

      auto opcode = GET_OPCODE(*i);
      if (opcode == OP_LOADK) {
         // rb = k + GETARG_Bx(i);
         auto arg_b = GETARG_Bx(*i);
         auto rb = IndexAt(typeDictionary()->PointerTo(luaTypes.TValue), ConstAddress((void*)(prototype->k)), ConstInt32(arg_b));

         // *ra = *rb;
         auto rb_value = LoadIndirect("TValue", "value_", rb);
         auto rb_tt = LoadIndirect("TValue", "tt_", rb);
         StoreIndirect("TValue", "value_", ra, rb_value);
         StoreIndirect("TValue", "tt_", ra, rb_tt);
      }
      else if (opcode == OP_RETURN) {
         auto arg_b = GETARG_B(*i);
         Store("arg_b", ConstInt32(arg_b));
         Store("arg_b",
               Call("luaD_poscall", 4, L, ci, ra, (arg_b != 0 ? ConstInt32(arg_b - 1) :
                                                   IndexAt(luaTypes.StkId,
                                                           LoadIndirect("lua_State", "top", L),
                                                           Sub(ConstInt32(0), ra)))));

         // Cheat: because of where the JIT dispatch happens in the VM, a JITed function can
         //        never be a fresh interpreter invocation. We can therefore safely skip the
         //        check and do the cleanup.
         //
         // Note:  because of where the JIT dispatch happens in the VM, the `ci = L->ci` is
         //        done by the interpreter immediately after the JIT/JITed function returns.
         TR::IlBuilder* resetTop = nullptr;
         IfThen(&resetTop, NotEqualTo(Load("arg_b"), ConstInt32(0)));
         resetTop->StoreIndirect("lua_State", "top", L, LoadIndirect("CallInfo", "top", ci));

         // Saving ci (into L->ci) is not needed as it is done for us by `luaD_poscall`
         Store("L", L);
         Return();
         return true;
      }
      else {
         break;
      }

      // pc++;
      auto pc = LoadIndirect("CallInfo", "u_l_savedpc", ci);
      auto newpc = IndexAt(typeDictionary()->PointerTo(luaTypes.Instruction), pc, ConstInt32(1));
      StoreIndirect("CallInfo", "u_l_savedpc", ci, newpc);
   }

   return false;
}
