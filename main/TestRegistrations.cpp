#include <rttr/registration>
#include <cstdio>

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