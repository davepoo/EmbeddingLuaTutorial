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

	{
		constexpr char* LUA_FILE = R"(
		function Pythagoras( a, b )
			return (a * a) + (b * b), a, b
		end
		)";

		lua_State* L = luaL_newstate();
		luaL_dostring(L, LUA_FILE);
		lua_getglobal(L, "Pythagoras");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, 3);
			lua_pushnumber(L, 4);
			constexpr int NUM_ARGS = 2;
			constexpr int NUM_RETURNS = 3;
			lua_pcall(L, NUM_ARGS, NUM_RETURNS, 0);
			lua_Number c = lua_tonumber(L, -3);
			printf("csqr = %d\n", (int)c);
			lua_Number a = lua_tonumber(L, -2);
			printf("a = %d\n", (int)a);
			lua_Number b = lua_tonumber(L, -1);
			printf("b = %d\n", (int)b);
		}
		lua_close(L);
	}

	{
		auto NativePythagoras = [](lua_State* L) -> int
		{
			lua_Number a = lua_tonumber(L, -2);
			lua_Number b = lua_tonumber(L, -1);
			lua_Number csqr = (a * a) + (b * b);
			lua_pushnumber(L, csqr);
			return 1;
		};

		constexpr char* LUA_FILE = R"(
		function Pythagoras( a, b )
			csqr = NativePythagoras( a, b )
			return csqr, a, b
		end
		)";

		lua_State* L = luaL_newstate();
		lua_pushcfunction(L, NativePythagoras);
		lua_setglobal(L, "NativePythagoras");
		luaL_dostring(L, LUA_FILE);
		lua_getglobal(L, "Pythagoras");
		if (lua_isfunction(L, -1))
		{
			lua_pushnumber(L, 3);
			lua_pushnumber(L, 4);
			constexpr int NUM_ARGS = 2;
			constexpr int NUM_RETURNS = 3;
			lua_pcall(L, NUM_ARGS, NUM_RETURNS, 0);
			lua_Number c = lua_tonumber(L, -3);
			printf("csqr = %d\n", (int)c);
			lua_Number a = lua_tonumber(L, -2);
			printf("a = %d\n", (int)a);
			lua_Number b = lua_tonumber(L, -1);
			printf("b = %d\n", (int)b);
		}
		lua_close(L);
	}
}