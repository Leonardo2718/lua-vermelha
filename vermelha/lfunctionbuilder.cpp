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
   setUseBytecodeBuilders();

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

   TR::BytecodeBuilder** bytecodeBuilders = new TR::BytecodeBuilder*[instructionCount];

   for (auto i = 0; i < instructionCount; ++i) {
      auto opcode = GET_OPCODE(instructions[i]);
      bytecodeBuilders[i] = OrphanBytecodeBuilder(i, (char*)luaP_opnames[opcode]);
   }

   Store("ci", LoadIndirect("lua_State", "ci", Load("L")));
   Store("base", LoadIndirect("CallInfo", "u_l_base", Load("ci")));

   // make the last return instruction (always inserted by the VM) a dummy fallthrough
   // that never gets taken to avoid a `treetops are not forming a single doubly linked list`
   // assertion failure
   auto dummy = OrphanBuilder();
   dummy->AppendBuilder(bytecodeBuilders[instructionCount - 1]);
   IfThen(&dummy, ConstInt32(0));

   AppendBuilder(bytecodeBuilders[0]);

   for (auto i = 0; i < instructionCount; ++i) {
      auto instruction = instructions[i];
      auto builder = bytecodeBuilders[i];

      auto nextBuilder = (i < instructionCount - 1) ? bytecodeBuilders[i + 1] : nullptr;

      // ra = base + GETARG_A(i)
      auto arg_a = GETARG_A(instruction);
      builder->Store("ra",
      builder->      IndexAt(luaTypes.StkId,
      builder->              Load("base"),
      builder->              ConstInt32(arg_a)));

      auto opcode = GET_OPCODE(instruction);
      if (opcode == OP_LOADK) {
         // rb = k + GETARG_Bx(i);
         auto arg_b = GETARG_Bx(instruction);
         builder->Store("rb",
         builder->   IndexAt(typeDictionary()->PointerTo(luaTypes.TValue),
         builder->           ConstAddress((void*)(prototype->k)),
         builder->           ConstInt32(arg_b)));

         // *ra = *rb;
         auto rb_value = builder->LoadIndirect("TValue", "value_", builder->Load("rb"));
         auto rb_tt = builder->LoadIndirect("TValue", "tt_", builder->Load("rb"));
         builder->StoreIndirect("TValue", "value_", builder->Load("ra"), rb_value);
         builder->StoreIndirect("TValue", "tt_", builder->Load("ra"), rb_tt);
      }
      else if (opcode == OP_RETURN) {
         auto arg_b = GETARG_B(instruction);
         // b = GETARG_B(i
         builder->Store("b",
         builder->      ConstInt32(arg_b));

         // b = luaD_poscall(L, ci, ra, (b != 0 ? b - 1 : cast_int(L->top - ra)))
         builder->Store("b",
         builder->      Call("luaD_poscall", 4,
         builder->           Load("L"),
         builder->           Load("ci"),
         builder->           Load("ra"),
                             (arg_b != 0 ?
         builder->                        ConstInt32(arg_b - 1) :
         builder->                                          IndexAt(luaTypes.StkId,
         builder->                                                  LoadIndirect("lua_State", "top",
         builder->                                                               Load("L")),
         builder->                                                  Sub(
         builder->                                                      ConstInt32(0),
         builder->                                                      Load("ra"))))));

         // Cheat: because of where the JIT dispatch happens in the VM, a JITed function can
         //        never be a fresh interpreter invocation. We can therefore safely skip the
         //        check and do the cleanup.
         //
         // Note:  because of where the JIT dispatch happens in the VM, the `ci = L->ci` is
         //        done by the interpreter immediately after the JIT/JITed function returns.

         // if (b) L->top = ci->top;
         TR::IlBuilder* resetTop = nullptr;
         builder->IfThen(&resetTop,
         builder->       NotEqualTo(
         builder->                  Load("b"),
         builder->                  ConstInt32(0)));

         resetTop->StoreIndirect("lua_State", "top",
         resetTop->              Load("L"),
         resetTop->              LoadIndirect("CallInfo", "top",
         resetTop->                           Load("ci")));

         builder->Return();
         nextBuilder = nullptr;   // prevent addition of a fallthrough path
      }
      else {
         return false;
      }

      // pc++;
      auto pc = builder->LoadIndirect("CallInfo", "u_l_savedpc", builder->Load("ci"));
      auto newpc = builder->IndexAt(typeDictionary()->PointerTo(luaTypes.Instruction),
                                    pc,
                   builder->        ConstInt32(1));
      builder->StoreIndirect("CallInfo", "u_l_savedpc", builder->Load("ci"), newpc);

      if (nextBuilder) builder->AddFallThroughBuilder(nextBuilder);
   }

   return true;
}
