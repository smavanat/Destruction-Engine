#pragma once
#ifndef MATHS_H
#define MATHS_H
#include"externals/Include/box2d/box2d.h"

static const double DEGREES_TO_RADIANS = ((2 * acos(0.0)) / 180);

//Could we not just make this a typdef of b2Vec2?
typedef b2Vec2 Vector2;

#ifdef __cplusplus
extern "C" {
#endif
//Rotates a Vector2 about the z-axis and returns the result. Takes an angle in degrees or in radians, but if 
//the angle is in degrees, the inRadians parameter must be set to false;
Vector2 rotate(Vector2* vec, double angle, bool inRadians);

//Rotates a Vector2 about a point in the z-axis and returns the result. Takes an angle in degrees or in radians, 
//but if the angle is in degrees, the inRadians parameter must be set to false.
Vector2 rotateAboutPoint(Vector2* point, Vector2* centre, double angle, bool inRadians);

Vector2 rotateAboutPoint2(Vector2* point, Vector2* centre, double angle, bool inRadians);

bool equals(Vector2 a, Vector2 b);

double normalizeAngle(double angle);

void normalise(Vector2* v);

#ifdef __cplusplus
}
#endif

#endif
