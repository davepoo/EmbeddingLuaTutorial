#include <cstdio>
#include "lua.hpp"

int main()
{
	{
		lua_State* L = luaL_newstate();
		luaL_dostring(L, "x = 47");
		lua_getglobal(L, "x");
		lua_Number x = lua_tonumber(L, 1);
		printf("lua says x = %d\n", (int)x);
		lua_close(L);
	}

	// nil
	// boolean
	// light userdata
	// number
	// string
	// table x = { "foo", "bar" }
	// function
	// userdata
	// thread

	{
		lua_State* L = luaL_newstate();
		lua_pushnumber(L, 42);
		lua_pushnumber(L, 52);
		lua_pushnumber(L, 62);

		// 42 - 1
		// 52 - 2
		// 62 - 3

		lua_Number x = lua_tonumber(L, 3);
		printf("lua says x = %d\n", (int)x);

		// 42 - -3
		// 52 - -2
		// 62 - -1

		lua_Number xx = lua_tonumber(L, -1);
		printf("the last thing i pushed was %d\n", (int)xx);

		lua_remove(L, 2);	//this should remove 52 from the stack

		// 42 - -2
		// 62 - -1

		// 42 - 1
		// 62 - 2

		lua_Number xxx = lua_tonumber(L, 2);
		printf("position 2 is %d\n", (int)xxx);

		lua_close(L);
	}
}