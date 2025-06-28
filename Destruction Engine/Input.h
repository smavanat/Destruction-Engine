#pragma once
struct Input {
	bool leftMouseButtonDown = false;
	float mouseX = 0.0f;
	float mouseY = 0.0f;

	Input() = default;

	Input(bool buttonDown, float mX, float mY) : leftMouseButtonDown(buttonDown), mouseX(mX), mouseY(mY) {}
};