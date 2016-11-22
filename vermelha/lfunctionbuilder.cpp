// vermelha headers
#include "lfunctionbuilder.hpp"

// c libraries
#include <cmath>
#include <stdio.h>

static void printAddr(unsigned char* addr) {
   printf("Compiled! (%p)\n", addr);
}

#define Protect(x)	{ {x;}; base = ci->u.l.base; }

/*
** some macros for common tasks in 'luaV_execute'
*/


#define RA(i)	(base+GETARG_A(i))
#define RB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgR, base+GETARG_B(i))
#define RC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgR, base+GETARG_C(i))
#define RKB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_B(i)) ? k+INDEXK(GETARG_B(i)) : base+GETARG_B(i))
#define RKC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_C(i)) ? k+INDEXK(GETARG_C(i)) : base+GETARG_C(i))


/* execute a jump instruction */
#define dojump(ci,i,e) \
  { int a = GETARG_A(i); \
    if (a != 0) luaF_close(L, ci->u.l.base + a - 1); \
    ci->u.l.savedpc += GETARG_sBx(i) + e; }

/* for test instructions, execute the jump instruction that follows it */
#define donextjump(ci)	{ i = *ci->u.l.savedpc; dojump(ci, i, 1); }


#define Protect(x)	{ {x;}; base = ci->u.l.base; }

#define checkGC(L,c)  \
	{ luaC_condGC(L, L->top = (c),  /* limit of live values */ \
                         Protect(L->top = ci->top));  /* restore top */ \
           luai_threadyield(L); }

/*
** copy of 'luaV_gettable', but protecting the call to potential
** metamethod (which can reallocate the stack)
*/
#define gettableProtected(L,t,k,v)  { const TValue *slot; \
  if (luaV_fastget(L,t,k,slot,luaH_get)) { setobj2s(L, v, slot); } \
  else Protect(luaV_finishget(L,t,k,v,slot)); }


/* same for 'luaV_settable' */
#define settableProtected(L,t,k,v) { const TValue *slot; \
  if (!luaV_fastset(L,t,k,slot,luaH_get,v)) \
    Protect(luaV_finishset(L,t,k,v,slot)); }


StkId jit_gettableProtected(lua_State* L, TValue* t, TValue* k, TValue* v) {
   auto ci = L->ci;
   StkId base = ci->u.l.base;
   gettableProtected(L, t, k, v);
   return base;
}

StkId jit_settableProtected(lua_State* L, TValue* t, TValue* k, TValue* v) {
   auto ci = L->ci;
   StkId base = ci->u.l.base;
   settableProtected(L, t, k, v);
   return base;
}

StkId vm_pow(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RKB(i);
   TValue *rc = RKC(i);
   lua_Number nb; lua_Number nc;
   if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
     setfltvalue(ra, luai_numpow(L, nb, nc));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_POW)); }

   // epilogue
   return base;
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

   DefineFunction("jit_gettableProtected", "0", "0", (void*)jit_gettableProtected,
                  luaTypes.StkId, 4,
                  plua_State,
                  pTValue,
                  pTValue,
                  pTValue);

   DefineFunction("jit_settableProtected", "0", "0", (void*)jit_settableProtected,
                  luaTypes.StkId, 4,
                  plua_State,
                  pTValue,
                  pTValue,
                  pTValue);

   DefineFunction("vm_pow", "0", "0", (void*)vm_pow,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);
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
   Store("base", LoadIndirect("CallInfo", "u.l.base", Load("ci"))); // base = ci->u.l.base

   // cl = clLvalue(ci->func)
   // cl = &(cast<GCUnion*>(ci->func->value_.gc)->cl.l)
   Store("cl",
         ConvertTo(typeDictionary()->PointerTo(luaTypes.LClosure),
                   LoadIndirect("TValue", "value_", // pretend `value_` is really `value_.gc` because it's a union
                                LoadIndirect("CallInfo", "func",
                                             Load("ci")))));

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
      case OP_GETTABUP:
         do_gettabup(builder, instruction);
         break;
      case OP_SETTABUP:
         do_settabup(builder, instruction);
         break;
      case OP_ADD:
         do_add(builder, instruction);
         break;
      case OP_SUB:
         do_sub(builder, instruction);
         break;
      case OP_MUL:
         do_mul(builder, instruction);
         break;
      case OP_POW:
         do_pow(builder, instruction);
         break;
      case OP_BAND:
         do_band(builder, instruction);
         break;
      case OP_BOR:
         do_bor(builder, instruction);
         break;
      case OP_BXOR:
         do_bxor(builder, instruction);
         break;
      case OP_IDIV:
         do_idiv(builder, instruction);
         break;
      case OP_SHL:
         do_shl(builder, instruction);
         break;
      case OP_SHR:
         do_shr(builder, instruction);
         break;
      case OP_UNM:
         do_unm(builder, instruction);
         break;
      case OP_BNOT:
         do_bnot(builder, instruction);
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
         auto pc = builder->LoadIndirect("CallInfo", "u.l.savedpc", builder->Load("ci"));
         auto newpc = builder->IndexAt(typeDictionary()->PointerTo(luaTypes.Instruction),
                                       pc,
                      builder->        ConstInt32(1));
         builder->StoreIndirect("CallInfo", "u.l.savedpc", builder->Load("ci"), newpc);

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

bool Lua::FunctionBuilder::do_gettabup(TR::BytecodeBuilder* builder, Instruction instruction) {
   // upval = cl->upvals[GETARG_B(instruction)]->v
   auto pUpVal = typeDictionary()->PointerTo(luaTypes.UpVal);
   auto ppUpVal = typeDictionary()->PointerTo(pUpVal);
   builder->Store("upval",
   builder->      LoadIndirect("UpVal", "v",
   builder->                   LoadAt(pUpVal,
   builder->                          IndexAt(ppUpVal,
   builder->                                  Add(
   builder->                                      Load("cl"),
   builder->                                      ConstInt32(typeDictionary()->OffsetOf("LClosure", "upvals"))),
   builder->                                  ConstInt32(GETARG_B(instruction))))));

   auto rc = jit_RK(GETARG_C(instruction), builder);

   builder->Store("base",
   builder->      Call("jit_gettableProtected", 4,
   builder->           Load("L"),
   builder->           Load("upval"),
                       rc,
   builder->           Load("ra")));

   return true;
}

bool Lua::FunctionBuilder::do_settabup(TR::BytecodeBuilder* builder, Instruction instruction) {
   // upval = cl->upvals[GETARG_A(instruction)]->v
   auto pUpVal = typeDictionary()->PointerTo(luaTypes.UpVal);
   auto ppUpVal = typeDictionary()->PointerTo(pUpVal);
   builder->Store("upval",
   builder->      LoadIndirect("UpVal", "v",
   builder->                   LoadAt(pUpVal,
   builder->                          IndexAt(ppUpVal,
   builder->                                  Add(
   builder->                                      Load("cl"),
   builder->                                      ConstInt32(typeDictionary()->OffsetOf("LClosure", "upvals"))),
   builder->                                  ConstInt32(GETARG_A(instruction))))));

   auto rb = jit_RK(GETARG_B(instruction), builder);
   auto rc = jit_RK(GETARG_C(instruction), builder);
   
   builder->Store("base",
   builder->      Call("jit_settableProtected", 4,
   builder->           Load("L"),
   builder->           Load("upval"),
                       rb,
                       rc));

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

bool Lua::FunctionBuilder::do_sub(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Sub(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_mul(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Mul(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

/*bool Lua::FunctionBuilder::do_mod(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Sub(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}*/

bool Lua::FunctionBuilder::do_pow(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_pow", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));
}

bool Lua::FunctionBuilder::do_idiv(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Div(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_band(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              And(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_bor(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Or(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_bxor(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Xor(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_shl(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              ShiftL(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_shr(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              ShiftR(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_unm(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("rb", jit_RK(GETARG_B(instruction), builder));   // rb = RKB(i);
   builder->Store("rc", jit_RK(GETARG_C(instruction), builder));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Sub(
   builder->                  ConstInt64(0),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));
}

bool Lua::FunctionBuilder::do_bnot(TR::BytecodeBuilder* builder, Instruction instruction) {
   // rb = RB(i);
   builder->Store("rb", 
   builder->      IndexAt(typeDictionary()->PointerTo(luaTypes.TValue),
   builder->              Load("base"),
   builder->              ConstInt32(GETARG_B(instruction))));

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Xor(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  ConstInt64(-1)));

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
   builder->      LoadIndirect("CallInfo", "u.l.base",
   builder->                   Load("ci")));
}
