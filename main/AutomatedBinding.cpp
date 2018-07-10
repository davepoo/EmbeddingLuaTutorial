#include "AutomatedBinding.h"
#include "ArenaAllocator.h"
#include "lua.hpp"
#include <cstdio>
#include <assert.h>

void AutomatedBindingTutorial()
{
	printf("---- automated binding using run time type info -----\n");

	//create memory pool for Lua
	constexpr int POOL_SIZE = 1024 * 20;
	char memory[POOL_SIZE];
	ArenaAllocator pool(memory, &memory[POOL_SIZE - 1]);

	//open the Lua state using our memory pool
	lua_State* L = lua_newstate(ArenaAllocator::l_alloc, &pool);

	/*! \brief The Lua script, you would probably load this data from a .lua file. */
	constexpr char* LUA_SCRIPT = R"(
		-- this is a lua script
		)";

	//execute the lua script
	int doResult = luaL_dostring(L, LUA_SCRIPT);
	if (doResult != LUA_OK)
	{
		printf("Error: %s\n", lua_tostring(L, -1));
	}

	//close the Lua state
	lua_close(L);
}