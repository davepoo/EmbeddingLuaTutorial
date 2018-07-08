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
		x = 47
		)";

	//execute the lua script
	int doResult = luaL_dostring(L, LUA_SCRIPT);
	if (doResult == LUA_OK)
	{
		lua_getglobal(L, "x");
		lua_Number x = lua_tonumber(L, 1);
		printf("lua says x = %d\n", (int)x);
	}
	else
	{
		printf("Error: %s\n", lua_tostring(L, -1));
	}

	//close the Lua state
	lua_close(L);
}

// *** Methods for Sprite class ***

Sprite::Sprite() : x(0), y(0) {}

void Sprite::Move(int velX, int velY)
{
	x += velX;
	y += velY;
}

void Sprite::Draw()
{
	printf("sprite(%p): x = %d, y = %d\n", this, x, y);
}
