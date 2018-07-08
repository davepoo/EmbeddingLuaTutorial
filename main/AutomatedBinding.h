#pragma once

/*! \brief This is a our Test/Tutorial class that we are binding to Lua */
class  Sprite
{
public:
	int x;
	int y;

	Sprite();
	~Sprite() = default;
	void Move(int velX, int velY);
	void Draw();
};

/*! \brief our main method for this tutorial, called from main() */
void AutomatedBindingTutorial();


