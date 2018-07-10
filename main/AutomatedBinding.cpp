#include "AutomatedBinding.h"
#include "ArenaAllocator.h"
#include "lua.hpp"
#include <cstdio>
#include <assert.h>
#include <rttr/registration>

/*! \brief The Lua script, you would probably load this data from a .lua file. */
constexpr char* LUA_SCRIPT = R"(
		-- this is a lua script
		Global.HelloWorld()
		Global.HelloWorld2()
		Global.HelloWorld3()
		)";

int CallGlobalFromLua(lua_State* L)
{
	rttr::method* m = (rttr::method*)lua_touserdata(L, lua_upvalueindex(1));
	rttr::method& methodToInvoke(*m);
	methodToInvoke.invoke({});
	return 0;
}

void AutomatedBindingTutorial()
{
	printf("---- automated binding using run time type info -----\n");

	//create memory pool for Lua
	constexpr int POOL_SIZE = 1024 * 20;
	char memory[POOL_SIZE];
	ArenaAllocator pool(memory, &memory[POOL_SIZE - 1]);

	//open the Lua state using our memory pool
	lua_State* L = lua_newstate(ArenaAllocator::l_alloc, &pool);

	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "Global");

	lua_pushvalue(L, -1);											//1
	for (auto& method : rttr::type::get_global_methods() )
	{
		lua_pushstring(L, method.get_name().to_string().c_str());	//2
		lua_pushlightuserdata(L, (void*)&method);
		lua_pushcclosure(L, CallGlobalFromLua, 1);					//3 
		lua_settable(L, -3);										//1[2] = 3
	}

	//execute the lua script
	int doResult = luaL_dostring(L, LUA_SCRIPT);
	if (doResult != LUA_OK)
	{
		printf("Error: %s\n", lua_tostring(L, -1));
	}

	//close the Lua state
	lua_close(L);
}