#pragma once
/*
* Wrapper class to tell other systems which SDL inputs have been activated. 
* Currently not very extensive since I don't need it to work for the full input range right now
*/
struct Input {
	bool leftMouseButtonDown = false;
	float mouseX = 0.0f;
	float mouseY = 0.0f;

	Input() = default;

	Input(bool buttonDown, float mX, float mY) : leftMouseButtonDown(buttonDown), mouseX(mX), mouseY(mY) {}
};