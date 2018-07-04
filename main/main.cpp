#include "ArenaAllocator.h"
#include <cstdio>
#include "lua.hpp"
#include <assert.h>
#include <string.h>
#include <new>


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

	// == What you should know by now ==
	// create and destroy lua state
	// get global numbers from lua
	// How to use the lua stack from c api
	// call lua functions from c
	// bind and call c functions from lua

	{
		struct Sprite
		{
			int x;
			int y;

			void Move(int velX, int velY)
			{
				x += velX;
				y += velY;
			}
		};

		auto CreateSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_newuserdata(L, sizeof(Sprite));
			sprite->x = 0;
			sprite->y = 0;
			return 1;
		};

		auto MoveSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			lua_Number velX = lua_tonumber(L, -2);
			lua_Number velY = lua_tonumber(L, -1);
			sprite->Move((int)velX, (int)velY);
			return 0;
		};

		constexpr char* LUA_FILE = R"(
		sprite = CreateSprite()
		MoveSprite( sprite, 5, 7 )
		)";

		lua_State* L = luaL_newstate();
		lua_pushcfunction(L, CreateSprite);
		lua_setglobal(L, "CreateSprite");
		lua_pushcfunction(L, MoveSprite);
		lua_setglobal(L, "MoveSprite");
		luaL_dostring(L, LUA_FILE);
		lua_getglobal(L, "sprite");
		if (lua_isuserdata(L, -1))
		{
			printf("We got a sprite from Lua\n");
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			printf("x = %d, y = %d\n", sprite->x, sprite->y);
		}
		else
		{
			printf("We DIDNT get a sprite from Lua\n");
		}
		lua_close(L);
	}

	printf("---- tables -----\n");

	{
		constexpr char* LUA_FILE = R"(
		x = { dave = "busy", ian = "idle" }
		)";

		lua_State* L = luaL_newstate();
		luaL_dostring(L, LUA_FILE);

		lua_getglobal(L, "x");
		lua_pushstring(L, "dave");
		lua_gettable(L, -2);
		const char* daveIs = lua_tostring(L, -1);
		printf("daveIs = %s\n", daveIs);

		lua_getglobal(L, "x");
		lua_getfield(L, -1, "ian");
		const char* ianIs = lua_tostring(L, -1);
		printf("ianIs = %s\n", ianIs);

		lua_getglobal(L, "x");
		lua_pushstring(L, "sleeping");
		lua_setfield(L, -2, "john");

		lua_getglobal(L, "x");
		lua_getfield(L, -1, "john");
		const char* johnIs = lua_tostring(L, -1);
		printf("johnIs = %s\n", johnIs);

		lua_close(L);
	}

	printf("---- metatables and metamethod(s) -----\n");
	{
		struct Vec
		{
			static int CreateVector2D(lua_State* L)
			{
				lua_newtable(L);
				lua_pushstring(L, "x");
				lua_pushnumber(L, 0);
				lua_settable(L, -3);

				lua_pushstring(L, "y");
				lua_pushnumber(L, 0);
				lua_settable(L, -3);

				luaL_getmetatable(L, "VectorMetaTable");
				lua_setmetatable(L, -2);

				return 1;
			}

			static int __add(lua_State* L)
			{
				printf("__add was called\n");
				assert(lua_istable(L, -2));		// left table
				assert(lua_istable(L, -1));		// right table

				lua_pushstring(L, "x");
				lua_gettable(L, -3);
				lua_Number xLeft = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_pushstring(L, "x");
				lua_gettable(L, -2);
				lua_Number xRight = lua_tonumber(L, -1);
				lua_pop(L, 1);

				lua_Number xAdded = xLeft + xRight;
				printf("xAdded = %d\n", (int)xAdded);

				Vec::CreateVector2D(L);
				lua_pushstring(L, "x");
				lua_pushnumber(L, xAdded);
				lua_rawset(L, -3);
				return 1;
			}
		};

		constexpr char* LUA_FILE = R"(
		v1 = CreateVector()   -- v1 is a table
		v2 = CreateVector()	  -- v2 is a table
		v1.x = 11
		v2.x = 42
		v3 = v1 + v2
		result = v3.x
		)";

		lua_State* L = luaL_newstate();

		lua_pushcfunction(L, Vec::CreateVector2D);
		lua_setglobal(L, "CreateVector");

		luaL_newmetatable(L, "VectorMetaTable");
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, Vec::__add);
		lua_settable(L, -3);

		int x = luaL_dostring(L, LUA_FILE);
		if (x != LUA_OK)
		{
			printf("Error: %s\n", lua_tostring(L, -1));
		}

		lua_getglobal(L, "result");
		lua_Number result = lua_tonumber(L, -1);
		printf("result = %d\n", (int)result);
		lua_close(L);
	}

	printf("---- C++ constructors and destructors -----\n");
	{
		static int numberOfSpritesExisting = 0;

		struct Sprite
		{
			int x;
			int y;

			Sprite() : x(0), y(0)
			{
				numberOfSpritesExisting++;
			}

			~Sprite()
			{
				numberOfSpritesExisting--;
			}

			void Move(int velX, int velY)
			{
				x += velX;
				y += velY;
			}

			void Draw()
			{
				printf("sprite(%p): x = %d, y = %d\n", this, x, y);
			}
		};

		auto CreateSprite = [](lua_State* L) -> int
		{
			void* pointerToASprite = lua_newuserdata(L, sizeof(Sprite));
			new (pointerToASprite) Sprite();
			luaL_getmetatable(L, "SpriteMetaTable");
			assert(lua_istable(L, -1));
			lua_setmetatable(L, -2);
			return 1;
		};

		auto DestroySprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->~Sprite();
			return 0;
		};

		auto MoveSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			lua_Number velX = lua_tonumber(L, -2);
			lua_Number velY = lua_tonumber(L, -1);
			sprite->Move((int)velX, (int)velY);
			return 0;
		};

		auto DrawSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->Draw();
			return 0;
		};

		constexpr char* LUA_FILE = R"(
		sprite = CreateSprite()
		MoveSprite( sprite, 5, 7 )
		DrawSprite( sprite )
		MoveSprite( sprite, 1, 2 )
		DrawSprite( sprite )

		sprite2 = CreateSprite()
		MoveSprite( sprite2, 3, 3 )
		DrawSprite( sprite2 )
		)";

		lua_State* L = luaL_newstate();

		luaL_newmetatable(L, "SpriteMetaTable");
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, DestroySprite);
		lua_settable(L, -3);

		lua_pushcfunction(L, CreateSprite);
		lua_setglobal(L, "CreateSprite");
		lua_pushcfunction(L, MoveSprite);
		lua_setglobal(L, "MoveSprite");
		lua_pushcfunction(L, DrawSprite);
		lua_setglobal(L, "DrawSprite");

		int doResult = luaL_dostring(L, LUA_FILE);
		if (doResult != LUA_OK)
		{
			printf("Error: %s\n", lua_tostring(L, -1));
		}

		lua_close(L);

		assert(numberOfSpritesExisting == 0);
	}

	printf("---- Object Oriented Access -----\n");
	{
		static int numberOfSpritesExisting = 0;

		struct Sprite
		{
			int x;
			int y;

			Sprite() : x(0), y(0)
			{
				numberOfSpritesExisting++;
			}

			~Sprite()
			{
				numberOfSpritesExisting--;
			}

			void Move(int velX, int velY)
			{
				x += velX;
				y += velY;
			}

			void Draw()
			{
				printf("sprite(%p): x = %d, y = %d\n", this, x, y);
			}
		};

		auto CreateSprite = [](lua_State* L) -> int
		{
			void* pointerToASprite = lua_newuserdata(L, sizeof(Sprite));
			new (pointerToASprite) Sprite();
			luaL_getmetatable(L, "SpriteMetaTable");
			assert(lua_istable(L, -1));
			lua_setmetatable(L, -2);
			return 1;
		};

		auto DestroySprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->~Sprite();
			return 0;
		};

		auto MoveSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			lua_Number velX = lua_tonumber(L, -2);
			lua_Number velY = lua_tonumber(L, -1);
			sprite->Move((int)velX, (int)velY);
			return 0;
		};

		auto DrawSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->Draw();
			return 0;
		};

		constexpr char* LUA_FILE = R"(
		sprite = Sprite.new()
		sprite:Move( 5, 7 )		-- Sprite.Move( sprite, 5, 7 )
		sprite:Draw()
		sprite:Move( 1, 2 )
		sprite:Draw()

		sprite2 = Sprite.new()
		sprite2:Move( 3, 3 )
		sprite2:Draw()

		-- sprite	-> sprite is a userdatum
		--		has a metatable called SpriteMetaTable
		--			dont have Move(), use the __index metamethod
		--			__index metamethod is a table which is Sprite
		--			Sprite has a field called Move(), invoke that
		--			Move() is a c function
		--			invoke, pass the userdatum as the first parameter.
		)";

		lua_State* L = luaL_newstate();

		lua_newtable(L);
		int spriteTableIdx = lua_gettop(L);
		lua_pushvalue(L, spriteTableIdx);
		lua_setglobal(L, "Sprite");

		lua_pushcfunction(L, CreateSprite);
		lua_setfield(L, -2, "new");
		lua_pushcfunction(L, MoveSprite);
		lua_setfield(L, -2, "Move");
		lua_pushcfunction(L, DrawSprite);
		lua_setfield(L, -2, "Draw");

		luaL_newmetatable(L, "SpriteMetaTable");
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, DestroySprite);
		lua_settable(L, -3);

		lua_pushstring(L, "__index");
		lua_pushvalue(L, spriteTableIdx);
		lua_settable(L, -3);

		int doResult = luaL_dostring(L, LUA_FILE);
		if (doResult != LUA_OK)
		{
			printf("Error: %s\n", lua_tostring(L, -1));
		}

		lua_close(L);

		assert(numberOfSpritesExisting == 0);
	}

	printf("---- Reading Object Properties -----\n");
	{
		static int numberOfSpritesExisting = 0;

		struct Sprite
		{
			int x;
			int y;

			Sprite() : x(0), y(0)
			{
				numberOfSpritesExisting++;
			}

			~Sprite()
			{
				numberOfSpritesExisting--;
			}

			void Move(int velX, int velY)
			{
				x += velX;
				y += velY;
			}

			void Draw()
			{
				printf("sprite(%p): x = %d, y = %d\n", this, x, y);
			}
		};

		auto CreateSprite = [](lua_State* L) -> int
		{
			void* pointerToASprite = lua_newuserdata(L, sizeof(Sprite));
			new (pointerToASprite) Sprite();
			luaL_getmetatable(L, "SpriteMetaTable");
			assert(lua_istable(L, -1));
			lua_setmetatable(L, -2);
			return 1;
		};

		auto DestroySprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->~Sprite();
			return 0;
		};

		auto MoveSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			lua_Number velX = lua_tonumber(L, -2);
			lua_Number velY = lua_tonumber(L, -1);
			sprite->Move((int)velX, (int)velY);
			return 0;
		};

		auto DrawSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->Draw();
			return 0;
		};

		auto SpriteIndex = [](lua_State* L) -> int
		{
			assert(lua_isuserdata(L, -2));
			assert(lua_isstring(L, -1));

			Sprite* sprite = (Sprite*)lua_touserdata(L, -2);
			const char* index = lua_tostring(L, -1);
			if (strcmp(index, "x") == 0)
			{
				lua_pushnumber(L, sprite->x);
				return 1;
			}
			else if (strcmp(index, "y") == 0)
			{
				lua_pushnumber(L, sprite->y);
				return 1;
			}
			else
			{
				lua_getglobal(L, "Sprite");
				lua_pushstring(L, index);
				lua_rawget(L, -2);
				return 1;
			}
		};

		constexpr char* LUA_FILE = R"(
		sprite = Sprite.new()
		sprite:Move( 6, 7 )		-- Sprite.Move( sprite, 6, 7 )
		sprite:Draw()
		temp_x = sprite.x
		)";

		lua_State* L = luaL_newstate();

		lua_newtable(L);
		int spriteTableIdx = lua_gettop(L);
		lua_pushvalue(L, spriteTableIdx);
		lua_setglobal(L, "Sprite");

		lua_pushcfunction(L, CreateSprite);
		lua_setfield(L, -2, "new");
		lua_pushcfunction(L, MoveSprite);
		lua_setfield(L, -2, "Move");
		lua_pushcfunction(L, DrawSprite);
		lua_setfield(L, -2, "Draw");

		luaL_newmetatable(L, "SpriteMetaTable");
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, DestroySprite);
		lua_settable(L, -3);

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, SpriteIndex);
		lua_settable(L, -3);

		int doResult = luaL_dostring(L, LUA_FILE);
		if (doResult != LUA_OK)
		{
			printf("Error: %s\n", lua_tostring(L, -1));
		}

		lua_getglobal(L, "temp_x");
		lua_Number temp_x = lua_tonumber(L, -1);
		assert(temp_x == 6);

		lua_close(L);

		assert(numberOfSpritesExisting == 0);
	}

	printf("---- Writing Object Properties -----\n");
	{
		static int numberOfSpritesExisting = 0;

		struct Sprite
		{
			int x;
			int y;

			Sprite() : x(0), y(0)
			{
				numberOfSpritesExisting++;
			}

			~Sprite()
			{
				numberOfSpritesExisting--;
			}

			void Move(int velX, int velY)
			{
				x += velX;
				y += velY;
			}

			void Draw()
			{
				printf("sprite(%p): x = %d, y = %d\n", this, x, y);
			}
		};

		auto CreateSprite = [](lua_State* L) -> int
		{
			void* pointerToASprite = lua_newuserdata(L, sizeof(Sprite));
			new (pointerToASprite) Sprite();
			luaL_getmetatable(L, "SpriteMetaTable");
			assert(lua_istable(L, -1));
			lua_setmetatable(L, -2);
			return 1;
		};

		auto DestroySprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->~Sprite();
			return 0;
		};

		auto MoveSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			lua_Number velX = lua_tonumber(L, -2);
			lua_Number velY = lua_tonumber(L, -1);
			sprite->Move((int)velX, (int)velY);
			return 0;
		};

		auto DrawSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->Draw();
			return 0;
		};

		auto SpriteIndex = [](lua_State* L) -> int
		{
			assert(lua_isuserdata(L, -2));
			assert(lua_isstring(L, -1));

			Sprite* sprite = (Sprite*)lua_touserdata(L, -2);
			const char* index = lua_tostring(L, -1);
			if (strcmp(index, "x") == 0)
			{
				lua_pushnumber(L, sprite->x);
				return 1;
			}
			else if (strcmp(index, "y") == 0)
			{
				lua_pushnumber(L, sprite->y);
				return 1;
			}
			else
			{
				lua_getglobal(L, "Sprite");
				lua_pushstring(L, index);
				lua_rawget(L, -2);
				return 1;
			}
		};

		auto SpriteNewIndex = [](lua_State* L) -> int
		{
			assert(lua_isuserdata(L, -3));
			assert(lua_isstring(L, -2));
			// -1 - value we want to set

			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			const char* index = lua_tostring(L, -2);
			if (strcmp(index, "x") == 0)
			{
				sprite->x = (int)lua_tonumber(L, -1);
			}
			else if (strcmp(index, "y") == 0)
			{
				sprite->y = (int)lua_tonumber(L, -1);
			}
			else
			{
				assert(false);	//don't want you to write to my native object!!
			}

			return 0;
		};

		constexpr char* LUA_FILE = R"(
		sprite = Sprite.new()
		sprite:Move( 6, 7 )		-- Sprite.Move( sprite, 6, 7 )
		sprite:Draw()
		sprite.y = 10
		temp_x = sprite.x
		sprite:Draw()
		)";

		lua_State* L = luaL_newstate();

		lua_newtable(L);
		int spriteTableIdx = lua_gettop(L);
		lua_pushvalue(L, spriteTableIdx);
		lua_setglobal(L, "Sprite");

		lua_pushcfunction(L, CreateSprite);
		lua_setfield(L, -2, "new");
		lua_pushcfunction(L, MoveSprite);
		lua_setfield(L, -2, "Move");
		lua_pushcfunction(L, DrawSprite);
		lua_setfield(L, -2, "Draw");

		luaL_newmetatable(L, "SpriteMetaTable");
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, DestroySprite);
		lua_settable(L, -3);

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, SpriteIndex);
		lua_settable(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, SpriteNewIndex);
		lua_settable(L, -3);

		int doResult = luaL_dostring(L, LUA_FILE);
		if (doResult != LUA_OK)
		{
			printf("Error: %s\n", lua_tostring(L, -1));
		}

		lua_getglobal(L, "temp_x");
		lua_Number temp_x = lua_tonumber(L, -1);
		assert(temp_x == 6);

		lua_close(L);

		assert(numberOfSpritesExisting == 0);
	}

	printf("---- user values -----\n");
	{
		static int numberOfSpritesExisting = 0;

		struct Sprite
		{
			int x;
			int y;

			Sprite() : x(0), y(0)
			{
				numberOfSpritesExisting++;
			}

			~Sprite()
			{
				numberOfSpritesExisting--;
			}

			void Move(int velX, int velY)
			{
				x += velX;
				y += velY;
			}

			void Draw()
			{
				printf("sprite(%p): x = %d, y = %d\n", this, x, y);
			}
		};

		auto CreateSprite = [](lua_State* L) -> int
		{
			void* pointerToASprite = lua_newuserdata(L, sizeof(Sprite));
			new (pointerToASprite) Sprite();
			luaL_getmetatable(L, "SpriteMetaTable");
			assert(lua_istable(L, -1));
			lua_setmetatable(L, -2);

			lua_newtable(L);
			lua_setuservalue(L, 1);

			return 1;
		};

		auto DestroySprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->~Sprite();
			return 0;
		};

		auto MoveSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			lua_Number velX = lua_tonumber(L, -2);
			lua_Number velY = lua_tonumber(L, -1);
			sprite->Move((int)velX, (int)velY);
			return 0;
		};

		auto DrawSprite = [](lua_State* L) -> int
		{
			Sprite* sprite = (Sprite*)lua_touserdata(L, -1);
			sprite->Draw();
			return 0;
		};

		auto SpriteIndex = [](lua_State* L) -> int
		{
			assert(lua_isuserdata(L, -2));	//1
			assert(lua_isstring(L, -1));	//2

			Sprite* sprite = (Sprite*)lua_touserdata(L, -2);
			const char* index = lua_tostring(L, -1);
			if (strcmp(index, "x") == 0)
			{
				lua_pushnumber(L, sprite->x);
				return 1;
			}
			else if (strcmp(index, "y") == 0)
			{
				lua_pushnumber(L, sprite->y);
				return 1;
			}
			else
			{
				lua_getuservalue(L, 1);
				lua_pushvalue(L, 2);
				lua_gettable(L, -2);
				if (lua_isnil(L, -1))
				{
					lua_getglobal(L, "Sprite");
					lua_pushstring(L, index);
					lua_rawget(L, -2);
				}
				return 1;
			}
		};

		auto SpriteNewIndex = [](lua_State* L) -> int
		{
			assert(lua_isuserdata(L, -3));  //1
			assert(lua_isstring(L, -2));	//2
			// -1 - value we want to set	//3

			Sprite* sprite = (Sprite*)lua_touserdata(L, -3);
			const char* index = lua_tostring(L, -2);
			if (strcmp(index, "x") == 0)
			{
				sprite->x = (int)lua_tonumber(L, -1);
			}
			else if (strcmp(index, "y") == 0)
			{
				sprite->y = (int)lua_tonumber(L, -1);
			}
			else
			{
				lua_getuservalue(L, 1);	//1
				lua_pushvalue(L, 2);	//2
				lua_pushvalue(L, 3);	//3
				lua_settable(L, -3);	//1[2] = 3
			}

			return 0;
		};

		constexpr char* LUA_FILE = R"(
		sprite = Sprite.new()
		sprite:Move( 6, 7 )		-- Sprite.Move( sprite, 6, 7 )
		sprite:Draw()
		sprite.y = 10
		sprite.zzz = 99
		sprite.x = sprite.zzz
		temp_x = sprite.x
		sprite:Draw()
		)";

		constexpr int POOL_SIZE = 1024 * 10;
		char memory[POOL_SIZE];
		ArenaAllocator pool(memory, &memory[POOL_SIZE - 1]);
		//GlobalAllocator pool;
		//for (int i = 0; i < 50000; i++)	//comment this line in for benchmarking the mem allocator
		{
			pool.Reset();
			lua_State* L = lua_newstate(ArenaAllocator::l_alloc, &pool);
			//lua_State* L = luaL_newstate();
			//lua_State* L = lua_newstate(GlobalAllocator::l_alloc, &pool);

			lua_newtable(L);
			int spriteTableIdx = lua_gettop(L);
			lua_pushvalue(L, spriteTableIdx);
			lua_setglobal(L, "Sprite");

			lua_pushcfunction(L, CreateSprite);
			lua_setfield(L, -2, "new");
			lua_pushcfunction(L, MoveSprite);
			lua_setfield(L, -2, "Move");
			lua_pushcfunction(L, DrawSprite);
			lua_setfield(L, -2, "Draw");

			luaL_newmetatable(L, "SpriteMetaTable");
			lua_pushstring(L, "__gc");
			lua_pushcfunction(L, DestroySprite);
			lua_settable(L, -3);

			lua_pushstring(L, "__index");
			lua_pushcfunction(L, SpriteIndex);
			lua_settable(L, -3);

			lua_pushstring(L, "__newindex");
			lua_pushcfunction(L, SpriteNewIndex);
			lua_settable(L, -3);

			int doResult = luaL_dostring(L, LUA_FILE);
			if (doResult != LUA_OK)
			{
				printf("Error: %s\n", lua_tostring(L, -1));
			}

			lua_getglobal(L, "temp_x");
			lua_Number temp_x = lua_tonumber(L, -1);
			assert(temp_x == 99);

			lua_close(L);
		}

		assert(numberOfSpritesExisting == 0);
	}

	printf("---- lua memory allocation -----\n");
	{
		struct LuaMem
		{
			static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
				(void)ud; (void)osize;  /* not used */
				if (nsize == 0) {
					free(ptr);
					return NULL;
				}
				else
					return realloc(ptr, nsize);
			}
		};

		void* ud = nullptr;
		lua_State* L = lua_newstate( LuaMem::l_alloc, ud );
		assert(L != nullptr);
		lua_close(L);
	}

	printf("---- lua arena memory allocation -----\n");
	{
		constexpr int POOL_SIZE = 1024 * 10;
		char memory[POOL_SIZE];
		ArenaAllocator pool(memory, &memory[POOL_SIZE-1] );
		lua_State* L = lua_newstate(ArenaAllocator::l_alloc, &pool);
		assert(L != nullptr);
		lua_close(L);
	}

	printf("---- lua arena aligned memory allocation -----\n");
	{
		constexpr int POOL_SIZE = 1024 * 10;
		char memory[POOL_SIZE];
		ArenaAllocator pool(memory, &memory[POOL_SIZE - 1]);
		lua_State* L = lua_newstate(ArenaAllocator::l_alloc, &pool);

		struct alignas(8) Thing
		{
			float x;
			float z;
		};

		Thing* t = (Thing*)lua_newuserdata(L, sizeof(Thing) );
		assert( (uintptr_t)t % alignof(Thing) == 0); //are we aligned???

		assert(L != nullptr);
		lua_close(L);
	}
}