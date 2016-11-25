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


void jit_setbvalue(TValue* obj, int x) {
   setbvalue(obj,x);
}

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

StkId vm_gettable(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   StkId rb = RB(i);
   TValue *rc = RKC(i);
   gettableProtected(L, rb, rc, ra);

   // epilogue
   return base;
}

StkId vm_settable(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RKB(i);
   TValue *rc = RKC(i);
   settableProtected(L, ra, rb, rc);

   // epilogue
   return base;
}

StkId vm_newtable(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   int b = GETARG_B(i);
   int c = GETARG_C(i);
   Table *t = luaH_new(L);
   sethvalue(L, ra, t);
   if (b != 0 || c != 0)
     luaH_resize(L, t, luaO_fb2int(b), luaO_fb2int(c));
   checkGC(L, ra + 1);

   // epilogue
   return base;
}

StkId vm_add(lua_State* L, Instruction i) {
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
   if (ttisinteger(rb) && ttisinteger(rc)) {
     lua_Integer ib = ivalue(rb); lua_Integer ic = ivalue(rc);
     setivalue(ra, intop(+, ib, ic));
   }
   else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
     setfltvalue(ra, luai_numadd(L, nb, nc));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_ADD)); }

   // epilogue
   return base;
}

StkId vm_sub(lua_State* L, Instruction i) {
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
   if (ttisinteger(rb) && ttisinteger(rc)) {
     lua_Integer ib = ivalue(rb); lua_Integer ic = ivalue(rc);
     setivalue(ra, intop(-, ib, ic));
   }
   else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
     setfltvalue(ra, luai_numsub(L, nb, nc));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_SUB)); }

   // epilogue
   return base;
}

StkId vm_mul(lua_State* L, Instruction i) {
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
   if (ttisinteger(rb) && ttisinteger(rc)) {
     lua_Integer ib = ivalue(rb); lua_Integer ic = ivalue(rc);
     setivalue(ra, intop(*, ib, ic));
   }
   else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
     setfltvalue(ra, luai_nummul(L, nb, nc));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_MUL)); }

   // epilogue
   return base;
}

StkId vm_mod(lua_State* L, Instruction i) {
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
   if (ttisinteger(rb) && ttisinteger(rc)) {
     lua_Integer ib = ivalue(rb); lua_Integer ic = ivalue(rc);
     setivalue(ra, luaV_mod(L, ib, ic));
   }
   else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
     lua_Number m;
     luai_nummod(L, nb, nc, m);
     setfltvalue(ra, m);
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_MOD)); }

   // epilogue
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

StkId vm_div(lua_State* L, Instruction i) {
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
     setfltvalue(ra, luai_numdiv(L, nb, nc));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_DIV)); }

   // epilogue
   return base;
}

StkId vm_idiv(lua_State* L, Instruction i) {
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
   if (ttisinteger(rb) && ttisinteger(rc)) {
     lua_Integer ib = ivalue(rb); lua_Integer ic = ivalue(rc);
     setivalue(ra, luaV_div(L, ib, ic));
   }
   else if (tonumber(rb, &nb) && tonumber(rc, &nc)) {
     setfltvalue(ra, luai_numidiv(L, nb, nc));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_IDIV)); }

   // epilogue
   return base;
}

StkId vm_band(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RKB(i);
   TValue *rc = RKC(i);
   lua_Integer ib; lua_Integer ic;
   if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
     setivalue(ra, intop(&, ib, ic));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_BAND)); }

   // epilogue
   return base;
}

StkId vm_bor(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RKB(i);
   TValue *rc = RKC(i);
   lua_Integer ib; lua_Integer ic;
   if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
     setivalue(ra, intop(|, ib, ic));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_BOR)); }

   // epilogue
   return base;
}

StkId vm_bxor(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RKB(i);
   TValue *rc = RKC(i);
   lua_Integer ib; lua_Integer ic;
   if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
     setivalue(ra, intop(^, ib, ic));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_BXOR)); }

   // epilogue
   return base;
}

StkId vm_shl(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RKB(i);
   TValue *rc = RKC(i);
   lua_Integer ib; lua_Integer ic;
   if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
     setivalue(ra, luaV_shiftl(ib, ic));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_SHL)); }

   // epilogue
   return base;
}

StkId vm_shr(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RKB(i);
   TValue *rc = RKC(i);
   lua_Integer ib; lua_Integer ic;
   if (tointeger(rb, &ib) && tointeger(rc, &ic)) {
     setivalue(ra, luaV_shiftl(ib, -ic));
   }
   else { Protect(luaT_trybinTM(L, rb, rc, ra, TM_SHR)); }

   // epilogue
   return base;
}

StkId vm_unm(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RB(i);
   lua_Number nb;
   if (ttisinteger(rb)) {
     lua_Integer ib = ivalue(rb);
     setivalue(ra, intop(-, 0, ib));
   }
   else if (tonumber(rb, &nb)) {
     setfltvalue(ra, luai_numunm(L, nb));
   }
   else {
     Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
   }

   // epilogue
   return base;
}

StkId vm_bnot(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RB(i);
   lua_Integer ib;
   if (tointeger(rb, &ib)) {
     setivalue(ra, intop(^, ~l_castS2U(0), ib));
   }
   else {
     Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
   }

   // epilogue
   return base;
}

StkId vm_not(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *rb = RB(i);
   int res = l_isfalse(rb);  /* next assignment may change this value */
   setbvalue(ra, res);

   // epilogue
   return base;
}

StkId vm_len(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   Protect(luaV_objlen(L, ra, RB(i)));

   // epilogue
   return base;
}

StkId vm_jmp(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   dojump(ci, i, 0);

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

   auto pTValue = types->PointerTo(luaTypes.TValue);
   auto plua_State = types->PointerTo(luaTypes.lua_State);

   DefineFunction((char*)"luaD_poscall", (char*)"ldo.c", (char*)"453",
                  (void*)luaD_poscall, Int32,
                  4,
                  types->PointerTo(luaTypes.lua_State),
                  types->PointerTo(luaTypes.CallInfo),
                  luaTypes.StkId,
                  Int32);
   DefineFunction((char*)"printAddr", (char*)"0", (char*)"0", (void*)printAddr, NoType, 1, Address);

   DefineFunction("jit_setbvalue", "0", "0", (void*)jit_setbvalue,
                  NoType, 2,
                  pTValue,
                  types->toIlType<int>());

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

   DefineFunction("vm_gettable", "0", "0", (void*)vm_gettable,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_settable", "0", "0", (void*)vm_settable,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_newtable", "0", "0", (void*)vm_newtable,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_add", "0", "0", (void*)vm_add,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_sub", "0", "0", (void*)vm_sub,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_mul", "0", "0", (void*)vm_mul,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_mod", "0", "0", (void*)vm_mod,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_pow", "0", "0", (void*)vm_pow,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_div", "0", "0", (void*)vm_div,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_idiv", "0", "0", (void*)vm_idiv,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_band", "0", "0", (void*)vm_band,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_bor", "0", "0", (void*)vm_bor,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_bxor", "0", "0", (void*)vm_bxor,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_shl", "0", "0", (void*)vm_shl,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_shr", "0", "0", (void*)vm_shr,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_unm", "0", "0", (void*)vm_unm,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_bnot", "0", "0", (void*)vm_bnot,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_not", "0", "0", (void*)vm_not,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_len", "0", "0", (void*)vm_len,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_jmp", "0", "0", (void*)vm_jmp,
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
      case OP_MOVE:
         do_move(builder, instruction);
         break;
      case OP_LOADK:
         do_loadk(builder, instruction);
         break;
      case OP_LOADBOOL:
         do_loadbool(builder, static_cast<TR::IlBuilder*>(bytecodeBuilders[i + 2]), instruction);
         break;
      case OP_LOADNIL:
         do_loadnil(builder, instruction);
         break;
      case OP_GETTABUP:
         do_gettabup(builder, instruction);
         break;
      case OP_GETTABLE:
         do_gettable(builder, instruction);
         break;
      case OP_SETTABUP:
         do_settabup(builder, instruction);
         break;
      case OP_SETTABLE:
         do_settable(builder, instruction);
         break;
      case OP_NEWTABLE:
         do_newtable(builder, instruction);
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
      case OP_MOD:
         do_mod(builder, instruction);
         break;
      case OP_POW:
         do_pow(builder, instruction);
         break;
      case OP_DIV:
         do_div(builder, instruction);
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
      case OP_NOT:
         do_not(builder, instruction);
         break;
      case OP_LEN:
         do_len(builder, instruction);
         break;
      case OP_JMP:
         do_jmp(builder, instruction);
         builder->AddFallThroughBuilder(bytecodeBuilders[i + 1 + GETARG_sBx(instruction)]);
         nextBuilder = nullptr;
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

bool Lua::FunctionBuilder::do_move(TR::BytecodeBuilder* builder, Instruction instruction) {
   // setobjs2s(L, ra, RB(i));
   jit_setobj(builder,
   builder->  Load("ra"),
              jit_R(builder, GETARG_B(instruction)));

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
   /*auto rb_value = builder->LoadIndirect("TValue", "value_", builder->Load("rb"));
   auto rb_tt = builder->LoadIndirect("TValue", "tt_", builder->Load("rb"));
   builder->StoreIndirect("TValue", "value_", builder->Load("ra"), rb_value);
   builder->StoreIndirect("TValue", "tt_", builder->Load("ra"), rb_tt);*/
   jit_setobj(builder,
              builder->Load("ra"),
              builder->Load("rb"));

   return true;
}

bool Lua::FunctionBuilder::do_loadbool(TR::BytecodeBuilder* builder, TR::IlBuilder* dest, Instruction instruction) {
   // setbvalue(ra, GETARG_B(i));
   builder->Call("jit_setbvalue", 2,
   builder->     Load("ra"),
   builder->     ConstInt32(GETARG_B(instruction)));

   // if (GETARG_C(i)) ci->u.l.savedpc++;
   builder->IfCmpNotEqualZero(&dest,
   builder->               ConstInt32(GETARG_C(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_loadnil(TR::BytecodeBuilder* builder, Instruction instruction) {
   auto setnils = OrphanBuilder();
   builder->ForLoopDown("b", &setnils,
   builder->            ConstInt32(GETARG_B(instruction) +1),
   builder->            ConstInt32(0),
   builder->            ConstInt32(1));

   setnils->StoreIndirect("TValue", "tt_",
   setnils->              Load("ra"),
   setnils->              ConstInt32(LUA_TNIL));
   setnils->IndexAt(typeDictionary()->PointerTo(luaTypes.TValue),
   setnils->        Load("ra"),
   setnils->        ConstInt32(1));

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

   auto rc = jit_RK(builder, GETARG_C(instruction));

   builder->Store("base",
   builder->      Call("jit_gettableProtected", 4,
   builder->           Load("L"),
   builder->           Load("upval"),
                       rc,
   builder->           Load("ra")));

   return true;
}

bool Lua::FunctionBuilder::do_gettable(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_gettable", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

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

   auto rb = jit_RK(builder, GETARG_B(instruction));
   auto rc = jit_RK(builder, GETARG_C(instruction));
   
   builder->Store("base",
   builder->      Call("jit_settableProtected", 4,
   builder->           Load("L"),
   builder->           Load("upval"),
                       rb,
                       rc));

   return true;
}

bool Lua::FunctionBuilder::do_settable(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_settable", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_newtable(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_newtable", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_add(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);*/

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

   /*builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Add(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_add", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_sub(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Sub(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_sub", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_mul(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Mul(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_mul", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_mod(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_mod", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_pow(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_pow", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_div(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_div", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_idiv(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Div(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_idiv", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_band(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              And(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_band", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_bor(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Or(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_bor", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_bxor(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Xor(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_bxor", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_shl(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              ShiftL(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_shl", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_shr(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              ShiftR(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rc"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_shr", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_unm(TR::BytecodeBuilder* builder, Instruction instruction) {
   /*builder->Store("rb", jit_RK(builder, GETARG_B(instruction)));   // rb = RKB(i);
   builder->Store("rc", jit_RK(builder, GETARG_C(instruction)));   // rc = RKC(i);

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Sub(
   builder->                  ConstInt64(0),
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb"))));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_unm", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_bnot(TR::BytecodeBuilder* builder, Instruction instruction) {
   // rb = RB(i);
   /*builder->Store("rb", jit_R(builder, GETARG_B(instruction)));

   builder->StoreIndirect("TValue", "value_",
   builder->              Load("ra"),
   builder->              Xor(
   builder->                  LoadIndirect("TValue", "value_",
   builder->                               Load("rb")),
   builder->                  ConstInt64(-1)));

   builder->StoreIndirect("TValue", "tt_",
   builder->             Load("ra"),
   builder->             ConstInt32(LUA_TNUMINT));*/
   builder->Store("base",
   builder->      Call("vm_bnot", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_not(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_not", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_len(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_len", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_jmp(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_jmp", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
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

void Lua::FunctionBuilder::jit_setobj(TR::BytecodeBuilder* builder, TR::IlValue* dest, TR::IlValue* src) {
   // *dest = *src;
   auto src_value = builder->LoadIndirect("TValue", "value_", src);
   auto src_tt = builder->LoadIndirect("TValue", "tt_", src);
   builder->StoreIndirect("TValue", "value_", dest, src_value);
   builder->StoreIndirect("TValue", "tt_", dest, src_tt);
}

TR::IlValue* Lua::FunctionBuilder::jit_R(TR::BytecodeBuilder* builder, int arg) {
   auto reg = builder->IndexAt(typeDictionary()->PointerTo(luaTypes.TValue),
              builder->        Load("base"),
              builder->        ConstInt32(arg));
   return reg;
}

TR::IlValue* Lua::FunctionBuilder::jit_RK(TR::BytecodeBuilder* builder, int arg) {
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
