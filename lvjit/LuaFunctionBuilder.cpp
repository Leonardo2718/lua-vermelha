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

// Lua Vermelha headers
#include "LuaFunctionBuilder.hpp"

// JitBuilder headers
#include "ilgen/VirtualMachineState.hpp"

// c libraries
#include <math.h>
#include <stdio.h>


//~ convenience functions to be called from JITed code ~~~~~~~~~~~~~~~~~~~~~~~~~

static void printAddr(void* addr) {
   printf("Compiled! (%p)\n", addr);
}


//~ Lua VM functions and macros ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
The following functions and macros are taken from `lvm.c`. These local copies
are used as helpers either by JITed code of IL generation.
*/

static int forlimit (const TValue *obj, lua_Integer *p, lua_Integer step,
                     int *stopnow) {
  *stopnow = 0;  /* usually, let loops run */
  if (!luaV_tointeger(obj, p, (step < 0 ? 2 : 1))) {  /* not fit in integer? */
    lua_Number n;  /* try to convert to float */
    if (!tonumber(obj, &n)) /* cannot convert to float? */
      return 0;  /* not a number */
    if (luai_numlt(0, n)) {  /* if true, float is larger than max integer */
      *p = LUA_MAXINTEGER;
      if (step < 0) *stopnow = 1;
    }
    else {  /* float is smaller than min integer */
      *p = LUA_MININTEGER;
      if (step >= 0) *stopnow = 1;
    }
  }
  return 1;
}

#define Protect(x)	{ {x;}; base = ci->u.l.base; }

#define RA(i)	(base+GETARG_A(i))
#define RB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgR, base+GETARG_B(i))
#define RC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgR, base+GETARG_C(i))
#define RKB(i)	check_exp(getBMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_B(i)) ? k+INDEXK(GETARG_B(i)) : base+GETARG_B(i))
#define RKC(i)	check_exp(getCMode(GET_OPCODE(i)) == OpArgK, \
	ISK(GETARG_C(i)) ? k+INDEXK(GETARG_C(i)) : base+GETARG_C(i))


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


//~ Lua VM macro wrappers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
The following are helper functions that wrap Lua VM macros so they can be called
from JITed code.
*/

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

//~ Lua VM interpreter helpers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
The following functions are helpers for handling specific opcodes. Each
implements a simplified version of the code in the Lua VM interpreter for
handling the coresponding opcode. Instead of generating IL for handling these
opcodes, JITed code can simply call these function, which essentially copy the
behavior of the the VM interpreter. Although this is not the most optimal
approach, it is simple and a good way of starting out. As the project evolves,
calls to these functions can be replaced by "hand-generated" IL.
*/

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

int32_t vm_test(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);
   int32_t skipnext = 0;

   // main body
   if (GETARG_C(i) ? l_isfalse(ra) : !l_isfalse(ra)) {
      ci->u.l.savedpc++;
      skipnext = 1;
   }
   else
      skipnext = 0;

   // epilogue
   return skipnext;
}

int32_t vm_testset(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);
   int32_t skipnext = 0;

   // main body
   TValue *rb = RB(i);
   if (GETARG_C(i) ? l_isfalse(rb) : !l_isfalse(rb)) {
      ci->u.l.savedpc++;
      skipnext = 1;
   }
   else {
      setobjs2s(L, ra, rb);
      skipnext = 0;
   }

   // epilogue
   return skipnext;
}

int32_t vm_forloop(lua_State* L, Instruction i) {
   // prologue
   int32_t continueLoop = 0;
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   if (ttisinteger(ra)) {  /* integer loop? */
      lua_Integer step = ivalue(ra + 2);
      lua_Integer idx = intop(+, ivalue(ra), step); /* increment index */
      lua_Integer limit = ivalue(ra + 1);
      if ((0 < step) ? (idx <= limit) : (limit <= idx)) {
        ci->u.l.savedpc += GETARG_sBx(i);  /* jump back */
        continueLoop = 1;
        chgivalue(ra, idx);  /* update internal index... */
        setivalue(ra + 3, idx);  /* ...and external index */
      }
   }
   else {  /* floating loop */
      lua_Number step = fltvalue(ra + 2);
      lua_Number idx = luai_numadd(L, fltvalue(ra), step); /* inc. index */
      lua_Number limit = fltvalue(ra + 1);
      if (luai_numlt(0, step) ? luai_numle(idx, limit)
                              : luai_numle(limit, idx)) {
        ci->u.l.savedpc += GETARG_sBx(i);  /* jump back */
        continueLoop = 1;
        chgfltvalue(ra, idx);  /* update internal index... */
        setfltvalue(ra + 3, idx);  /* ...and external index */
      }
   }

   // epilogue
   return continueLoop;
}

void vm_forprep(lua_State* L, Instruction i) {
   // prologue
   CallInfo *ci = L->ci;
   LClosure *cl = clLvalue(ci->func);
   TValue *k = cl->p->k;
   StkId base = ci->u.l.base;
   StkId ra = RA(i);

   // main body
   TValue *init = ra;
   TValue *plimit = ra + 1;
   TValue *pstep = ra + 2;
   lua_Integer ilimit;
   int stopnow;
   if (ttisinteger(init) && ttisinteger(pstep) &&
       forlimit(plimit, &ilimit, ivalue(pstep), &stopnow)) {
     /* all values are integer */
     lua_Integer initv = (stopnow ? 0 : ivalue(init));
     setivalue(plimit, ilimit);
     setivalue(init, intop(-, initv, ivalue(pstep)));
   }
   else {  /* try making all values floats */
     lua_Number ninit; lua_Number nlimit; lua_Number nstep;
     if (!tonumber(plimit, &nlimit))
       luaG_runerror(L, "'for' limit must be a number");
     setfltvalue(plimit, nlimit);
     if (!tonumber(pstep, &nstep))
       luaG_runerror(L, "'for' step must be a number");
     setfltvalue(pstep, nstep);
     if (!tonumber(init, &ninit))
       luaG_runerror(L, "'for' initial value must be a number");
     setfltvalue(init, luai_numsub(L, ninit, nstep));
   }
   ci->u.l.savedpc += GETARG_sBx(i);

   // epilogue
}


//~ general IL generation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Lua::FunctionBuilder::FunctionBuilder(Proto* p, Lua::TypeDictionary* types)
   : TR::MethodBuilder(types), prototype(p), luaTypes(types->getLuaTypes()) {

   char file_name_buff[1024];
   snprintf(file_name_buff, 1024, "%s", getstr(prototype->source));

   char line_num_buff[1024];
   snprintf(line_num_buff, 1024, "%d", prototype->linedefined);

   // since Lua functions do not have actual names, use the format
   // `<filename>:<first line of definition>_<last line of definition>`
   // as the "unique" function name
   char func_name_buff[1024];
   snprintf(func_name_buff, 1024, "%s:%s_%d", file_name_buff, line_num_buff, prototype->lastlinedefined);

   DefineLine(line_num_buff);
   DefineFile(file_name_buff);

   DefineName(func_name_buff);
   DefineParameter("L", types->PointerTo("lua_State"));
   DefineReturnType(NoType);
   setUseBytecodeBuilders();

   auto pTValue = types->PointerTo(luaTypes.TValue);
   auto plua_State = types->PointerTo(luaTypes.lua_State);

   // convenience functions to be called from JITed code

   DefineFunction("printAddr", __FILE__, "0", (void*)printAddr,
                  NoType, 1,
                  types->toIlType<void*>());

   DefineFunction("compiledbody", "0", "0", nullptr,
                  NoType, 1,
                  plua_State);

   // Lua VM functions to be called from JITed code

   DefineFunction("luaD_precall", "ldo.c", "344", (void*)luaD_precall,
                  Int32, 3,
                  plua_State,
                  luaTypes.StkId,
                  Int32);

   DefineFunction("luaD_poscall", "ldo.c", "453", (void*)luaD_poscall,
                  Int32, 4,
                  types->PointerTo(luaTypes.lua_State),
                  types->PointerTo(luaTypes.CallInfo),
                  luaTypes.StkId,
                  Int32);

   DefineFunction("luaV_execute", "lvm.c", "790", (void*)luaV_execute,
                  NoType, 1,
                  plua_State);

   DefineFunction("luaJ_compile", "lvjit.cpp", "53", (void*)luaJ_compile,
                  Int32, 1,
                  types->PointerTo(luaTypes.Proto));

   DefineFunction("luaF_close", "lfunc.c", "83", (void*)luaF_close,
                  NoType, 2,
                  plua_State,
                  luaTypes.StkId);

   DefineFunction("luaV_equalobj", "lvm.c", "409", (void*)luaV_equalobj,
                  types->toIlType<int>(), 3,
                  plua_State,
                  pTValue,
                  pTValue);

   DefineFunction("luaV_lessthan", "lvm.c", "366", (void*)luaV_lessthan,
                  types->toIlType<int>(), 3,
                  plua_State,
                  pTValue,
                  pTValue);

   DefineFunction("luaV_lessequal", "lvm.c", "386", (void*)luaV_lessequal,
                  types->toIlType<int>(), 3,
                  plua_State,
                  pTValue,
                  pTValue);

   // Lua VM macro wrappers to be called from JITed code

   DefineFunction("jit_setbvalue", __FILE__, "0", (void*)jit_setbvalue,
                  NoType, 2,
                  pTValue,
                  types->toIlType<int>());

   DefineFunction("jit_gettableProtected", __FILE__, "0", (void*)jit_gettableProtected,
                  luaTypes.StkId, 4,
                  plua_State,
                  pTValue,
                  pTValue,
                  pTValue);

   DefineFunction("jit_settableProtected", __FILE__, "0", (void*)jit_settableProtected,
                  luaTypes.StkId, 4,
                  plua_State,
                  pTValue,
                  pTValue,
                  pTValue);

   // Lua VM interpreter helpers

   DefineFunction("vm_gettable", __FILE__, "0", (void*)vm_gettable,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_settable", __FILE__, "0", (void*)vm_settable,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_newtable", __FILE__, "0", (void*)vm_newtable,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_add", __FILE__, "0", (void*)vm_add,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_sub", __FILE__, "0", (void*)vm_sub,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_mul", __FILE__, "0", (void*)vm_mul,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_mod", __FILE__, "0", (void*)vm_mod,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_pow", __FILE__, "0", (void*)vm_pow,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_div", __FILE__, "0", (void*)vm_div,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_idiv", __FILE__, "0", (void*)vm_idiv,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_band", __FILE__, "0", (void*)vm_band,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_bor", __FILE__, "0", (void*)vm_bor,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_bxor", __FILE__, "0", (void*)vm_bxor,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_shl", __FILE__, "0", (void*)vm_shl,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_shr", __FILE__, "0", (void*)vm_shr,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_unm", __FILE__, "0", (void*)vm_unm,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_bnot", __FILE__, "0", (void*)vm_bnot,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_not", __FILE__, "0", (void*)vm_not,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_len", __FILE__, "0", (void*)vm_len,
                  luaTypes.StkId, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_test", __FILE__, "0", (void*)vm_test,
                  Int32, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_testset", __FILE__, "0", (void*)vm_testset,
                  Int32, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_forloop", __FILE__, "0", (void*)vm_forloop,
                  Int32, 2,
                  plua_State,
                  luaTypes.Instruction);

   DefineFunction("vm_forprep", __FILE__, "0", (void*)vm_forprep,
                  NoType, 2,
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

   Store("cl",jit_clLvalue(this,
                                LoadIndirect("CallInfo", "func",
                                             Load("ci"))));

   setVMState(new OMR::VirtualMachineState{});
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
      case OP_EQ:
         do_cmp("luaV_equalobj", builder, static_cast<TR::IlBuilder*>(bytecodeBuilders[i + 2]), instruction);
         break;
      case OP_LT:
         do_cmp("luaV_lessthan", builder, static_cast<TR::IlBuilder*>(bytecodeBuilders[i + 2]), instruction);
         break;
      case OP_LE:
         do_cmp("luaV_lessequal", builder, static_cast<TR::IlBuilder*>(bytecodeBuilders[i + 2]), instruction);
         break;
      case OP_TEST:
         do_test(builder, static_cast<TR::IlBuilder*>(bytecodeBuilders[i + 2]), instruction);
         break;
      case OP_TESTSET:
         do_testset(builder, static_cast<TR::IlBuilder*>(bytecodeBuilders[i + 2]), instruction);
         break;
      case OP_CALL:
         do_call(builder, instruction);
         break;
      case OP_RETURN:
         do_return(builder, instruction);
         nextBuilder = nullptr;   // prevent addition of a fallthrough path
         break;
      case OP_FORLOOP:
         do_forloop(builder, static_cast<TR::IlBuilder*>(bytecodeBuilders[i + 1 + GETARG_sBx(instruction)]), instruction);
         break;
      case OP_FORPREP:
         do_forprep(builder, instruction);
         builder->AddFallThroughBuilder(bytecodeBuilders[i + 1 + GETARG_sBx(instruction)]);
         nextBuilder = nullptr;   // prevent addition of a fallthrough path
         break;
      default:
         return false;
      }

      if (nextBuilder) {
         builder->setVMState(new OMR::VirtualMachineState{});
         builder->AddFallThroughBuilder(nextBuilder);
      }
   }

   return true;
}


//~ opcode specific IL generation ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
   builder->           ConstAddress(prototype->k),
   builder->           ConstInt32(arg_b)));

   // *ra = *rb;
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
   builder->                                      ConstInt64(typeDictionary()->OffsetOf("LClosure", "upvals"))),
   builder->                                  ConstInt64(GETARG_B(instruction))))));

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
   builder->                                      ConstInt64(typeDictionary()->OffsetOf("LClosure", "upvals"))),
   builder->                                  ConstInt64(GETARG_A(instruction))))));

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
   builder->Store("base",
   builder->      Call("vm_add", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_sub(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_sub", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_mul(TR::BytecodeBuilder* builder, Instruction instruction) {
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
   builder->Store("base",
   builder->      Call("vm_idiv", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_band(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_band", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_bor(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_bor", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_bxor(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_bxor", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_shl(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_shl", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_shr(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_shr", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_unm(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Store("base",
   builder->      Call("vm_unm", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_bnot(TR::BytecodeBuilder* builder, Instruction instruction) {
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
#ifdef FULL_IL_GEN
   builder->Store("a",
   builder->      ConstInt32(GETARG_A(instruction)));

   TR::IlBuilder* call_luaF_close = OrphanBuilder();
   builder->IfThen(&call_luaF_close,
   builder->       NotEqualTo(
   builder->               Load("a"),
   builder->               ConstInt32(0)));

   call_luaF_close->Call("luaF_close", 2,
   call_luaF_close->     Load("L"),
   call_luaF_close->     Add(
   call_luaF_close->         LoadIndirect("CallInfo", "u.l.base",
   call_luaF_close->                      Load("ci")),
   call_luaF_close->         Sub(
   call_luaF_close->             Load("a"),
   call_luaF_close->             ConstInt32(1))));
#else
   int arg_a = GETARG_A(instruction);
   if (arg_a != 0) {
      builder->Call("luaF_close", 2,
      builder->     Load("L"),
      builder->     Add(
      builder->         LoadIndirect("CallInfo", "u.l.base",
      builder->                      Load("ci")),
      builder->         ConstInt32(arg_a - 1)));
   }
#endif

   return true;
}

bool Lua::FunctionBuilder::do_cmp(const char* cmpFunc, TR::BytecodeBuilder* builder, TR::IlBuilder* dest, Instruction instruction) {
   /* cmp can be "luaV_equalobj", "luaV_lessthan", or "luaV_lessequal" */

   // cmp =  cmpFunc(L, RKB(i), RKC(i));
   builder->Store("cmp",
   builder->      Call(cmpFunc, 3,
   builder->           Load("L"),
                       jit_RK(builder, GETARG_B(instruction)),
                       jit_RK(builder, GETARG_C(instruction))));

   jit_Protect(builder); // from code inspection it appears like the comparison call
                         // is the only thing that needs to be Protected because it
                         // calls `luaT_callTM`, which can reallocat the stack

   // if (cmp != GETARG_A(i)) ci->u.l.savedpc++;
   builder->IfCmpNotEqual(&dest,
   builder->              Load("cmp"),
   builder->              ConstInt32(GETARG_A(instruction)));

   return true;
}

bool Lua::FunctionBuilder::do_test(TR::BytecodeBuilder* builder, TR::IlBuilder* dest, Instruction instruction) {
   builder->Store("cmp",
   builder->      Call("vm_test", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   builder->IfCmpNotEqualZero(&dest,
   builder->              Load("cmp"));

   return true;
}

bool Lua::FunctionBuilder::do_testset(TR::BytecodeBuilder* builder, TR::IlBuilder* dest, Instruction instruction) {
   builder->Store("cmp",
   builder->      Call("vm_testset", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   builder->IfCmpNotEqualZero(&dest,
   builder->              Load("cmp"));

   return true;
}

bool Lua::FunctionBuilder::do_call(TR::BytecodeBuilder* builder, Instruction instruction) {
   // b = GETARG_B(i);
   builder->Store("b",
   builder->      ConstInt32(GETARG_B(instruction)));

   // nresults = GETARG_C(i) - 1;
   builder->Store("nresults",
   builder->      Sub(
   builder->          ConstInt32(GETARG_C(instruction)),
   builder->          ConstInt32(1)));

   // if (b != 0) L->top = ra+b;
   TR::IlBuilder* settop = builder->OrphanBuilder();
   builder->IfThen(&settop,
   builder->       NotEqualTo(
   builder->                  Load("b"),
   builder->                  ConstInt32(0)));

   settop->StoreIndirect("lua_State", "top",
   settop->               Load("L"),
   settop->               IndexAt(luaTypes.StkId,
   settop->                   Load("ra"),
   settop->                   Load("b")));

   // if (luaD_precall(L, ra, nresults))
   auto isCFunc = builder->Call("luaD_precall", 3,
                  builder->     Load("L"),
                  builder->     Load("ra"),
                  builder->     Load("nresults"));

   TR::IlBuilder* cFunc = builder->OrphanBuilder();
   TR::IlBuilder* luaFunc = builder->OrphanBuilder();
   builder->IfThenElse(&cFunc, &luaFunc, isCFunc);

   // if (nresults >= 0) L->top = ci->top;
   // Protect((void)0);
   TR::IlBuilder* adjusttop = cFunc->OrphanBuilder();
   cFunc->IfThen(&adjusttop,
   cFunc->       Or(
   cFunc->          GreaterThan(
   cFunc->                      Load("nresults"),
   cFunc->                      ConstInt32(0)),
   cFunc->          EqualTo(
   cFunc->                  Load("nresults"),
   cFunc->                  ConstInt32(0))));

   adjusttop->StoreIndirect("lua_State", "top",
   adjusttop->              Load("L"),
   adjusttop->              LoadIndirect("CallInfo", "top",
   adjusttop->                           Load("ci")));

   jit_Protect(cFunc);

   // p = getproto(L->ci->func);
   luaFunc->Store("p",
   luaFunc->      LoadIndirect("LClosure", "p",jit_clLvalue(luaFunc,
   luaFunc->                   LoadIndirect("CallInfo", "func",
   luaFunc->                                LoadIndirect("lua_State", "ci",
   luaFunc->                                             Load("L"))))));

   // f (!(p->jitflags & LUA_JITBLACKLIST) &&  p->callcounter == 0 &&  p->compiledcode == NULL)
   auto notblacklisted =
   luaFunc->EqualTo(
   luaFunc->        And(
   luaFunc->            LoadIndirect("Proto", "jitflags",
   luaFunc->                         Load("p")),
   luaFunc->            ConstInt32(LUA_JITBLACKLIST)),
   luaFunc->        ConstInt32(0));
   auto callcounter0 =
   luaFunc->EqualTo(
   luaFunc->        LoadIndirect("Proto", "callcounter",
   luaFunc->                     Load("p")),
   luaFunc->        ConstInt16(0));
   auto isnotcompiled =
   luaFunc->EqualTo(
   luaFunc->        LoadIndirect("Proto", "compiledcode",
   luaFunc->                     Load("p")),
   luaFunc->        ConstInt32(0));
   auto readycompile =
   luaFunc->And(notblacklisted,
   luaFunc->    And(callcounter0, isnotcompiled));
   TR::IlBuilder* trycompile = luaFunc->OrphanBuilder();
   luaFunc->IfThen(&trycompile, readycompile);

   // luaJ_compile(p);
   trycompile->Call("luaJ_compile", 1,
   trycompile->     Load("p"));

   // LUAJ_BLACKLIST(p); // p->jitflags = p->jitflags | LUA_JITBLACKLIST
   trycompile->StoreIndirect("Proto", "jitflags",
   trycompile->              Load("p"),
   trycompile->              Or(
   trycompile->                 LoadIndirect("Proto", "jitflags",
   trycompile->                           Load("p")),
   trycompile->                 ConstInt32(LUA_JITBLACKLIST)));

   // if (p->compiledcode) { p->compiledcode(L); }
   TR::IlBuilder* callcompiled = luaFunc->OrphanBuilder();
   TR::IlBuilder* callinterpreter = luaFunc->OrphanBuilder();
   luaFunc->IfThenElse(&callcompiled, &callinterpreter,
   luaFunc->           NotEqualTo(
   luaFunc->           LoadIndirect("Proto", "compiledcode",
   luaFunc->                        Load("p")),
   luaFunc->           ConstInt32(0)));

   callcompiled->ComputedCall("compiledbody", 2,
   callcompiled->             LoadIndirect("Proto", "compiledcode",
   callcompiled->                          Load("p")),
   callcompiled->             Load("L"));

   // else { luaV_execute(L); }
   callinterpreter->Call("luaV_execute", 1,
   callinterpreter->     Load("L"));

   return true;
}

bool Lua::FunctionBuilder::do_return(TR::BytecodeBuilder* builder, Instruction instruction) {
   // b = GETARG_B(i)
   auto arg_b = GETARG_B(instruction);
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

bool Lua::FunctionBuilder::do_forloop(TR::BytecodeBuilder* builder, TR::IlBuilder* loopStart, Instruction instruction) {
   builder->Store("continueLoop",
   builder->      Call("vm_forloop", 2,
   builder->           Load("L"),
   builder->           ConstInt32(instruction)));

   builder->IfCmpNotEqualZero(&loopStart,
   builder->              Load("continueLoop"));

   return true;
}

bool Lua::FunctionBuilder::do_forprep(TR::BytecodeBuilder* builder, Instruction instruction) {
   builder->Call("vm_forprep", 2,
   builder->     Load("L"),
   builder->     ConstInt32(instruction));

   return true;
}


//~ IL generation helpers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
                     builder->        ConstAddress(prototype->k),
                     builder->        ConstInt32(INDEXK(arg)))
                   :
                     builder->IndexAt(typeDictionary()->PointerTo(luaTypes.TValue),
                     builder->        Load("base"),
                     builder->        ConstInt32(arg));
}

TR::IlValue* Lua::FunctionBuilder::jit_clLvalue(TR::IlBuilder* builder, TR::IlValue* func) {
   // &(cast<GCUnion*>(func->value_.gc)->cl.l)
   return builder->ConvertTo(typeDictionary()->PointerTo(luaTypes.LClosure),
          builder->          LoadIndirect("TValue", "value_", func));  // pretend `value_` is really `value_.gc` because it's a union
}

void Lua::FunctionBuilder::jit_Protect(TR::IlBuilder* builder) {
   builder->Store("base",
   builder->      LoadIndirect("CallInfo", "u.l.base",
   builder->                   Load("ci")));
}
