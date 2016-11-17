#include "ltypedictionary.hpp"
#include "luavm.hpp"

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
   luaTypes.TValue = DEFINE_STRUCT(TValue);
   DEFINE_FIELD(TValue, value_, Int64); // this should actually be a union
   DEFINE_FIELD(TValue, tt_, Int32);
   //DefineField("TValue", "__padding0__", Int32);
   CLOSE_STRUCT(TValue);

   luaTypes.StkId = PointerTo("TValue"); // stack index

   // struct LClosure
   luaTypes.LClosure = DEFINE_STRUCT(LClosure);
   DEFINE_FIELD(LClosure, next, pGCObject_t);           // ClosureHeader  CommonHeader
   DEFINE_FIELD(LClosure, tt, luaTypes.lu_byte);        //      |              |
   DEFINE_FIELD(LClosure, marked, luaTypes.lu_byte);    //      |              |
   DEFINE_FIELD(LClosure, nupvalues, luaTypes.lu_byte); //      |
   DEFINE_FIELD(LClosure, gclist, pGCObject_t);         //      |
   DEFINE_FIELD(LClosure, p, pProto_t);
   DEFINE_FIELD(LClosure, upvals, Address);
   CLOSE_STRUCT(LClosure);

   // lfunc.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   luaTypes.UpVal = DEFINE_STRUCT(UpVal);
   DEFINE_FIELD(UpVal, v, PointerTo(luaTypes.TValue));  // points to stack or to its own value
   DEFINE_FIELD(UpVal, refcount, Int64);                // reference counter
/*
   union {
      struct {                                          // (when open)
         UpVal* next;                                   // linked list
         int touched;                                   // mark to avoid cycles with dead threads
      } open;
*/
   DefineField("UpVal", "u_value", luaTypes.TValue, offsetof(UpVal, u.value)); /* TValue value; */ // the value (when closed)
/*
   } u;
*/
   CLOSE_STRUCT(UpVal);

   // lstate.h types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   // struct CallInfo (Information about a call)
   luaTypes.CallInfo = DEFINE_STRUCT(CallInfo);
   TR::IlType* pCallInfo_t = PointerTo("CallInfo");
   DEFINE_FIELD(CallInfo, func, luaTypes.StkId);  // function index in the stack
   DEFINE_FIELD(CallInfo, top, luaTypes.StkId);   // top for this function
   DEFINE_FIELD(CallInfo, previous, pCallInfo_t); // dynamic call link
   DEFINE_FIELD(CallInfo, next, pCallInfo_t);     //       |
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
   DEFINE_FIELD(CallInfo, extra, PtrDiff_t);
   DEFINE_FIELD(CallInfo, nresults, Int16);
   DEFINE_FIELD(CallInfo, callstatus, luaTypes.lu_byte);
   CLOSE_STRUCT(CallInfo);

   // struct lua_State (per thread state)
   luaTypes.lua_State = DEFINE_STRUCT(lua_State);
   TR::IlType* pLuaState_t = PointerTo("lua_State");
   DEFINE_FIELD(lua_State, next, pGCObject_t);             // CommonHeader
   DEFINE_FIELD(lua_State, tt, luaTypes.lu_byte);          //      |
   DEFINE_FIELD(lua_State, marked, luaTypes.lu_byte);      //      |
   DEFINE_FIELD(lua_State, nci, Int16);                    // number of items in `ci` list (should be uint16_t ?)
   DEFINE_FIELD(lua_State, status, luaTypes.lu_byte);
   DEFINE_FIELD(lua_State, top, luaTypes.StkId);           // first free slot in the stack
   DEFINE_FIELD(lua_State, l_G, pGlobalState_t);
   DEFINE_FIELD(lua_State, ci, pCallInfo_t);               // call info for current function
   DEFINE_FIELD(lua_State, oldpc, pInstruction);           // last pc traced
   DEFINE_FIELD(lua_State, stack_last, luaTypes.StkId);    // last free slot in the stack
   DEFINE_FIELD(lua_State, stack, luaTypes.StkId);         // stack base
   DEFINE_FIELD(lua_State, openupval, pUpVal_t);           // list of open upvalues in this stack
   DEFINE_FIELD(lua_State, gclist, pGCObject_t);
   DEFINE_FIELD(lua_State, twups, pLuaState_t);            // list of threads with open upvalues
   DEFINE_FIELD(lua_State, errorJmp, pLuaLongjmp);         // current error recover point
   DEFINE_FIELD(lua_State, base_ci, luaTypes.CallInfo);    // CallInfo for first level (C calling Lua)
   DEFINE_FIELD(lua_State, hook, lua_Hook_t);              // (should be `volatile` ?)
   DEFINE_FIELD(lua_State, errfunc, PtrDiff_t);
   DEFINE_FIELD(lua_State, stacksize, Int32);
   DEFINE_FIELD(lua_State, basehookcount, Int32);
   DEFINE_FIELD(lua_State, hookcount, Int32);
   DEFINE_FIELD(lua_State, nny, Int16);                    // number of non-yieldable calls in stack
   DEFINE_FIELD(lua_State, nCcalls, Int16);                // number of nested C calls
   DEFINE_FIELD(lua_State, hookmask, l_signalT_t);
   DEFINE_FIELD(lua_State, allowhook, luaTypes.lu_byte);
   CLOSE_STRUCT(lua_State);
}