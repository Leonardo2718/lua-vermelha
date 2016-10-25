// vermelha headers
#include "lfunctionbuilder.hpp"
#include <stdio.h>

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

   auto pTValue = types->PointerTo(luaTypes.TValue);
   auto plua_State = types->PointerTo(luaTypes.lua_State);

   /*DefineFunction((char*)"settableProtected", (char*)"lfunction.cpp", (char*)"9",
                  (void*)settableProtected,
                  NoType,
                  4,
                  luaTypes.lua_State, pTValue, pTValue, pTValue);*/

   DefineFunction((char*)"luaV_finishget", (char*)"0", (char*)"0", (void*)luaV_finishget, NoType, 5,
                  plua_State, pTValue, pTValue, luaTypes.StkId, pTValue);
                  
   DefineFunction((char*)"luaV_finishset", (char*)"0", (char*)"0", (void*)luaV_finishset, NoType, 5,
                  plua_State, pTValue, pTValue, luaTypes.StkId, pTValue);
   
   /*DefineFunction((char*)"luaH_get", (char*)"0", (char*)"0", (void*)luaH_get, pTValue, 2,
                  pTable, pTValue);*/
}

bool Lua::FunctionBuilder::buildIL() {
   auto instructionCount = prototype->sizecode;
   auto instructions = prototype->code;

   TR::BytecodeBuilder** bytecodeBuilders = new TR::BytecodeBuilder*[instructionCount];

   for (auto i = 0; i < instructionCount; ++i) {
      auto opcode = GET_OPCODE(instructions[i]);
      bytecodeBuilders[i] = OrphanBytecodeBuilder(i, (char*)luaP_opnames[opcode]);
   }

   Store("ci", LoadIndirect("lua_State", "ci", Load("L")));         // ci = L->ci
   Store("base", LoadIndirect("CallInfo", "u_l_base", Load("ci"))); // base = ci->u.l.base

   // cl = clLvalue(ci->func)
   // cl = &(cast<GCUnion*>(ci->func->value_->gc)->cl.l)
   Store("cl",
         LoadAt(typeDictionary()->PointerTo(luaTypes.LClosure),
                ConvertTo(typeDictionary()->PointerTo(luaTypes.LClosure),
                          LoadIndirect("TValue", "value_", // pretend `value_` is really `value_->gc` because it's a union
                                        LoadIndirect("CallInfo", "func",
                                                     Load("ci"))))));

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

      switch (GET_OPCODE(instruction)) {
      case OP_LOADK:
         do_loadk(builder, instruction);
         break;
      case OP_ADD:
         do_add(builder, instruction);
         break;
      case OP_RETURN:
         do_return(builder, instruction);
         nextBuilder = nullptr;   // prevent addition of a fallthrough path
         break;
      default:
         return false;
      }

      if (nextBuilder) {
         // pc++;
         auto pc = builder->LoadIndirect("CallInfo", "u_l_savedpc", builder->Load("ci"));
         auto newpc = builder->IndexAt(typeDictionary()->PointerTo(luaTypes.Instruction),
                                       pc,
                      builder->        ConstInt32(1));
         builder->StoreIndirect("CallInfo", "u_l_savedpc", builder->Load("ci"), newpc);

         builder->AddFallThroughBuilder(nextBuilder);
      }
   }

   return true;
}

bool Lua::FunctionBuilder::do_loadk(TR::BytecodeBuilder* builder, Instruction instruction) {
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

   return true;
}

bool Lua::FunctionBuilder::do_settabup(TR::BytecodeBuilder* builder, Instruction instruction) {
   // upval = cl->upvals[GETARG_A(i)]->v
   builder->Store("upval",
   builder->      LoadIndirect("UpVal", "v",
   builder->                   LoadAt(typeDictionary()->PointerTo(luaTypes.UpVal),
   builder->                          LoadIndirect("LClosure", "upvals",
   builder->                                       Load("cl")))));

   auto rb = jit_RK(GETARG_B(instruction), builder);
   auto rc = jit_RK(GETARG_C(instruction), builder);
   

   builder->Call("settableProtected", 4, builder->Load("L"), builder->Load("upval"), rb, rc);

   builder->Store("base",
   builder->      LoadIndirect("CallInfo", "u_l_base",
   builder->                   Load("ci")));

   return true;
}

bool Lua::FunctionBuilder::do_add(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   /*
   TR::IlBuilder* integers = nullptr;
   TR::IlBuilder* notIntegers = nullptr;
   TR::IlBuilder* numbers = nullptr;
   TR::IlBuilder* notNumbers = nullptr;

   builder->IfThenElse(integers, notIntegers,
   builder->           And(
   builder->               EqualTo(
   builder->                       LoadIndirect("TValue", "tt_",
   builder->                                    Load("rb")),
   builder->                       ConstInt32(LUA_TNUMINT)),
   builder->               EqualTo(
   builder->                       LoadIndirect("TValue", "tt_",
   builder->                                    Load("rc")),
   builder->                       ConstInt32(LUA_TNUMINT))));
   */

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Add(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_return(TR::BytecodeBuilder* builder, Instruction instruction) {
   auto arg_b = GETARG_B(instruction);
   // b = GETARG_B(i)
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
   builder->                        IndexAt(luaTypes.StkId,
   builder->                                LoadIndirect("lua_State", "top",
   builder->                                             Load("L")),
   builder->                                Sub(
   builder->                                    ConstInt32(0),
   builder->                                    Load("ra"))))));

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
   
   return true;
}

void Lua::FunctionBuilder::jit_setobj(TR::BytecodeBuilder* builder, TR::IlValue* obj1, TR::IlValue* obj2) {
   // *obj1 = *obj2;
   auto rb_value = builder->LoadIndirect("TValue", "value_", obj2);
   auto rb_tt = builder->LoadIndirect("TValue", "tt_", obj2);
   builder->StoreIndirect("TValue", "value_", obj1, rb_value);
   builder->StoreIndirect("TValue", "tt_", obj1, rb_tt);
}

TR::IlValue* Lua::FunctionBuilder::jit_RK(int arg, TR::BytecodeBuilder* builder) {
   return ISK(arg) ?
                     builder->IndexAt(typeDictionary()->PointerTo(luaTypes.TValue),
                     builder->        ConstAddress((void*)(prototype->k)),
                     builder->        ConstInt32(INDEXK(arg)))
                   :
                     builder->IndexAt(typeDictionary()->PointerTo(luaTypes.TValue),
                     builder->        Load("base"),
                     builder->        ConstInt32(arg));
}

void Lua::FunctionBuilder::jit_Protect(TR::BytecodeBuilder* builder) {
   builder->Store("base",
   builder->      LoadIndirect("CallInfo", "u_l_base",
   builder->                   Load("ci")));
}

void Lua::FunctionBuilder::jit_gettableProtected(TR::BytecodeBuilder* builder, TR::IlValue* t, TR::IlValue* k, TR::IlValue* v) {
   
}

void Lua::FunctionBuilder::jit_fastget(TR::BytecodeBuilder* builder) {
   
}

void Lua::FunctionBuilder::jit_settableProtected(TR::BytecodeBuilder* builder, TR::IlValue* t, TR::IlValue* k, TR::IlValue* v) {
   #if 0 // a failed attempt
   // const TValue *slot;
   builder->Store("slot", builder->NullAddress());

   TR::IlBuilder* setSlotNull = nullptr;
   TR::IlBuilder* tryGetSlot = nullptr;
   builder->IfThenElse(&setSlotNull, &tryGetSlot, /*!ttistable(t)*/);

   // if (ttistable(t)) { // if (t->tt_ == LUA_TTABLE | BIT_ISCOLLECTABLE) {
   setSlotNull->Store("slot", builder->NullAddress());
   // } else {
   tryGetSlot->
   // }
   #endif
}

void Lua::FunctionBuilder::jit_fastset(TR::BytecodeBuilder* builder) {
}
