#include "ltypedictionary.hpp"
#include "luavm.hpp"
#include <stdio.h>
Lua::TypeDictionary::TypeDictionary() : TR::TypeDictionary() {
   // common lua types
   luaTypes.lu_byte         = Int8;
   luaTypes.lua_Integer     = Int32;
   luaTypes.lua_Unsigned    = luaTypes.lua_Integer;
   luaTypes.lua_Number      = Float;
   luaTypes.Instruction     = luaTypes.lua_Integer;

   // placeholder and convenience types
   auto pGCObject_t = Address;
   auto pGlobalState_t = Address; // state of all threads
   auto pUpVal_t = Address;
   auto pLuaLongjmp = Address;
   auto lua_Hook_t = Address;     // is actually `void(*)(lua_State*, lua_Debug*)
   auto PtrDiff_t = pInt8;        // is actually `ptrdiff_t`
   auto Sig_Atomic_t = Int32;     // is actually `sig_atomic_t`
   auto l_signalT_t = Sig_Atomic_t;
   auto lua_KContext_t = Address;
   auto pInstruction = PointerTo(luaTypes.Instruction);
   auto pProto_t = Address;

   // lobject.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   // struct TValue
   luaTypes.TValue = DefineStruct("TValue");
   DefineField("TValue", "value_", Int64); // this should actually be a union
   DefineField("TValue", "tt_", Int32);
   DefineField("TValue", "__padding0__", Int32);
   CloseStruct("TValue");

   luaTypes.StkId = PointerTo("TValue"); // stack index

   // struct LClosure
   luaTypes.LClosure = DefineStruct("LClosure");
   DefineField("LClosure", "next", pGCObject_t, offsetof(LClosure, next));                // ClosureHeader  CommonHeader
   DefineField("LClosure", "tt", luaTypes.lu_byte, offsetof(LClosure, tt));               //      |              |
   DefineField("LClosure", "marked", luaTypes.lu_byte, offsetof(LClosure, marked));       //      |              |
   DefineField("LClosure", "nupvalues", luaTypes.lu_byte, offsetof(LClosure, nupvalues)); //      |
   DefineField("LClosure", "gclist", pGCObject_t, offsetof(LClosure, gclist));            //      |
   DefineField("LClosure", "p", pProto_t, offsetof(LClosure, p));
   DefineField("LClosure", "upvals", Address, offsetof(LClosure, upvals));
   CloseStruct("LClosure");

   // lfunc.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   luaTypes.UpVal = DefineStruct("UpVal");
   DefineField("UpVal", "v", PointerTo(luaTypes.TValue), offsetof(UpVal, v));  // points to stack or to its own value
   DefineField("UpVal", "refcount", Int64, offsetof(UpVal, refcount));         // reference counter
/*
   union {
      struct {                                                                 // (when open)
         UpVal* next;                                                          // linked list
         int touched;                                                          // mark to avoid cycles with dead threads
      } open;
*/
   DefineField("UpVal", "u_value", luaTypes.TValue, offsetof(UpVal, u.value)); /* TValue value; */ // the value (when closed)
/*
   } u;
*/
   CloseStruct("UpVal", sizeof(UpVal));

   // lstate.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   // struct CallInfo (Information about a call)
   luaTypes.CallInfo = DefineStruct("CallInfo");
   TR::IlType* pCallInfo_t = PointerTo("CallInfo");
   DefineField("CallInfo", "func", luaTypes.StkId);  // function index in the stack
   DefineField("CallInfo", "top", luaTypes.StkId);   // top for this function
   DefineField("CallInfo", "previous", pCallInfo_t); // dynamic call link
   DefineField("CallInfo", "next", pCallInfo_t);     //       |
/*
   union { // Node: JitBuilder does not currently support defining unions
       struct {   // only for Lua functions
*/
           DefineField("CallInfo", "u_l_base", luaTypes.StkId);   /* StkId base; */ // base for this function
           DefineField("CallInfo", "u_l_savedpc", pInstruction);  /* const Instruction* savedpc; */
/*
       } l;
       struct {   // only for C functions
           lua_KFunction k;         // continuation in case of yields
           ptrdiff_t old_errfunc;
*/
           DefineField("CallInfo", "__padding__u_c_ctx", lua_KContext_t); /* lua_KContext ctx;*/ // context info. in case of yields
/*
       } c;
   } u;
*/
   DefineField("CallInfo", "extra", PtrDiff_t);
   DefineField("CallInfo", "nresults", Word);
   DefineField("CallInfo", "callstatus", luaTypes.lu_byte);
   CloseStruct("CallInfo");

   // struct lua_State (per thread state)
   luaTypes.lua_State = DefineStruct("lua_State");
   TR::IlType* pLuaState_t = PointerTo("lua_State");
   DefineField("lua_State", "next", pGCObject_t);             // CommonHeader
   DefineField("lua_State", "tt", luaTypes.lu_byte);          //      |
   DefineField("lua_State", "marked", luaTypes.lu_byte);      //      |
   DefineField("lua_State", "nci", Int16);                    // number of items in `ci` list (should be uint16_t ?)
   DefineField("lua_State", "status", luaTypes.lu_byte);
   DefineField("lua_State", "top", luaTypes.StkId);           // first free slot in the stack
   DefineField("lua_State", "l_G", pGlobalState_t);
   DefineField("lua_State", "ci", pCallInfo_t);               // call info for current function
   DefineField("lua_State", "oldpc", pInstruction);           // last pc traced
   DefineField("lua_State", "stack_last", luaTypes.StkId);    // last free slot in the stack
   DefineField("lua_State", "stack", luaTypes.StkId);         // stack base
   DefineField("lua_State", "openupval", pUpVal_t);           // list of open upvalues in this stack
   DefineField("lua_State", "gclist", pGCObject_t);
   DefineField("lua_State", "twups", pLuaState_t);            // list of threads with open upvalues
   DefineField("lua_State", "errorJmp", pLuaLongjmp);         // current error recover point
   DefineField("lua_State", "base_ci", luaTypes.CallInfo);    // CallInfo for first level (C calling Lua)
   DefineField("lua_State", "lua_Hook", lua_Hook_t);          // (should be `volatile` ?)
   DefineField("lua_State", "errfunc", PtrDiff_t);
   DefineField("lua_State", "stacksize", Int32);
   DefineField("lua_State", "basehookcount", Int32);
   DefineField("lua_State", "hookcount", Int32);
   DefineField("lua_State", "nny", Int16);                    // number of non-yieldable calls in stack
   DefineField("lua_State", "nCcalls", Int16);                // number of nested C calls
   DefineField("lua_State", "hookmask", l_signalT_t);
   DefineField("lua_State", "allowhook", luaTypes.lu_byte);
   CloseStruct("lua_State");
}