#include "Maths.h"
#include<stdio.h>
#include <stdlib.h>

//Rotates a Vector2 about the z-axis and returns the result. Takes an angle in degrees or in radians, but if 
//the angle is in degrees, the inRadians parameter must be set to false;
Vector2 rotate(Vector2* vec, double angle, bool inRadians) {
	Vector2 ret = {};
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (int)(round(vec->x * cos(angle) - vec->y * sin(angle)));
	ret.y = (int)(round(vec->x * sin(angle) + vec->y * cos(angle)));
	return ret;
}

//Rotates a Vector2 about a point in the z-axis and returns the result. Takes an angle in degrees or in radians, 
//but if the angle is in degrees, the inRadians parameter must be set to false.
Vector2 rotateAboutPoint(Vector2* point, Vector2* centre, double angle, bool inRadians) {
	Vector2 ret = {};
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (centre->x + (point->x - centre->x) * cos(angle) - (point->y - centre->y) * sin(angle));
	ret.y = (centre->y + (point->x - centre->x) * sin(angle) + (point->y - centre->y) * cos(angle));
	return ret;
}

Vector2 rotateAboutPoint2(Vector2* point, Vector2* centre, double angle, bool inRadians) {
	Vector2 ret = {};
	angle = fmod(angle, 360);
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (centre->x + (point->x - centre->x) * cos(angle) - (point->y - centre->y) * sin(angle));
	ret.y = (centre->y + (point->x - centre->x) * sin(angle) + (point->y - centre->y) * cos(angle));
	return ret;
}

double normalizeAngle(double angle) {
	if (angle < 0) {
		return angle + 2 * (2 * acos(0.0));  // Shift negative angles into [0, 2pi]
	}
	return angle;
}

void normalise(Vector2* v) {
	float mag = sqrt((v->x*v->x)+(v->y*v->y));

	v->x /= mag;
	v->y /= mag;
}

bool equals(Vector2 a, Vector2 b) {
	return fabs(a.x - b.x) < 0.001f && fabs(a.y - b.y) < 0.001f;
}