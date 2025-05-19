#pragma once
#include "Maths.h"
#include "Component.h"
#include "Texture.hpp"
#include "box2d/base.h"
struct Transform : public Component<Transform> {
	Vector2 position;
	double rotation;
};

struct Sprite : public Component<Sprite> {
	Texture* t;
};

struct Collider : public Component<Collider> {
	b2BodyId colliderId;
};