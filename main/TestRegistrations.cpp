#include <rttr/registration>
#include <cstdio>
#include "ArenaAllocator.h"
#include "AutomatedBinding.h"

// This Cpp file contains the stuff we are going to 
//Register with RTTR and will be bound to Lua.
//NOTE: this file nor any of the contents does not need to 
//be directly included/referenced in our Lua bindings.

void HelloWorld()
{
	printf("Hello, World\n");
}

void HelloWorld2()
{
	printf("Hello, World 2\n");
}

void HelloWorld3( int x, short y, int z )
{
	printf("Hello, World 3 (%d, %d, %d)\n", x, y, z);
}

short Add(short a, short b)
{
	return a + b;
}

short Mul(short a, short b)
{
	return a * b;
}

struct Sprite
{
	int x;
	int y;

	Sprite() : x(0), y(0) {}
	~Sprite() {}

	int Move(int velX, int velY)
	{
		x += velX;
		y += velY;
		return x + y;
	}

	void Draw()
	{
		printf("sprite(%p): x = %d, y = %d\n", this, x, y);
	}
};

RTTR_REGISTRATION
{
	rttr::registration::method("HelloWorld", &HelloWorld);
	rttr::registration::method("HelloWorld2", &HelloWorld2);
	rttr::registration::method("HelloWorld3", &HelloWorld3);
	rttr::registration::method("Add", &Add);
	rttr::registration::method("Mul", &Mul);
	rttr::registration::class_<Sprite>("Sprite")
		.constructor()
		.method("Move", &Sprite::Move)
		.method("Draw", &Sprite::Draw)
		.property("x", &Sprite::x)
		.property("y", &Sprite::y);
}

/*! \brief The Lua script, you would probably load this data from a .lua file. */
constexpr char* LUA_SCRIPT = R"(
		-- this is a lua script
		Global.HelloWorld()
		Global.HelloWorld2()
		local c = Global.Add( 42, 43 )
		local d = Global.Mul( c, 2 )
		Global.HelloWorld3( d, 99, 111 )
		local spr = Sprite.new()
		local xplusy = spr:Move( 1, 2 )
		local x = spr.x
		spr:Move( xplusy + x, xplusy )
		spr:Draw()
		spr.zzzz = 42
		local z = spr.zzzz
		spr:Move( z, z )
		spr:Draw()
		spr.x = 10
		spr:Draw()

		function Foo3( x, y, z )
			Global.HelloWorld3( x, y, z )	
		end

		function Foo2( x, y )
			Global.HelloWorld3( x, y, y )	
		end

		function Foo1( x )
			Global.HelloWorld3( x, x, x )	
		end

		function Foo( )
			Global.HelloWorld3( 42, 44, 43 )	
		end

		)";

/*! \brief This is our test application for lua binding with RTTR */
void AutomatedBindingTutorial()
{
	printf( "---- automated binding using run time type info -----\n" );

	//create memory pool for Lua
	constexpr int POOL_SIZE = 1024 * 20;
	char memory[POOL_SIZE];
	ArenaAllocator pool( memory, &memory[POOL_SIZE - 1] );

	//Create our Lua Script
	lua_State* L = CreateScript( pool );

	//load & execute the lua script
	LoadScript( L, LUA_SCRIPT );
	if ( ExecuteScript( L ) != LUA_OK )
	{
		printf( "Error: %s\n", lua_tostring( L, -1 ) );
	}

	//call our script functions
	CallScriptFunction( L, "Foo3", 1, 2, 3 );
	CallScriptFunction( L, "Foo2", 1, 2 );
	CallScriptFunction( L, "Foo1", 1 );
	CallScriptFunction( L, "Foo" );

	//close the Lua state
	CloseScript( L );
}