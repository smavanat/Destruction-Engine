#include "Maths.h"
#include<stdio.h>
#include<iostream>

//Rotates a Vector3 about the z-axis and returns the result. Takes an angle in degrees or in radians, but if 
//the angle is in degrees, the inRadians parameter must be set to false;
Vector3 rotate(Vector3 vec, double angle, bool inRadians) {
	Vector3 ret = {};
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = static_cast<int>(std::round(vec.x * cos(angle) - vec.y * sin(angle)));
	ret.y = static_cast<int>(std::round(vec.x * sin(angle) + vec.y * cos(angle)));
	ret.z = vec.z;
	return ret;
}

//Rotates a Vector2 about the z-axis and returns the result. Takes an angle in degrees or in radians, but if 
//the angle is in degrees, the inRadians parameter must be set to false;
Vector2 rotate(Vector2 vec, double angle, bool inRadians) {
	Vector2 ret = {};
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = static_cast<int>(std::round(vec.x * cos(angle) - vec.y * sin(angle)));
	ret.y = static_cast<int>(std::round(vec.x * sin(angle) + vec.y * cos(angle)));
	return ret;
}

//Rotates a Vector2 about a point in the z-axis and returns the result. Takes an angle in degrees or in radians, 
//but if the angle is in degrees, the inRadians parameter must be set to false.
Vector2 rotateAboutPoint(Vector2 point, Vector2 centre, double angle, bool inRadians) {
	Vector2 ret = {};
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (centre.x + (point.x - centre.x) * cos(angle) - (point.y - centre.y) * sin(angle));
	ret.y = (centre.y + (point.x - centre.x) * sin(angle) + (point.y - centre.y) * cos(angle));
	return ret;
}

b2Vec2 rotateAboutPoint2(Vector2 point, Vector2 centre, double angle, bool inRadians) {
	b2Vec2 ret = {};
	angle = fmod(angle, 360);
	if (!inRadians) {
		angle *= DEGREES_TO_RADIANS;
	}
	ret.x = (centre.x + (point.x - centre.x) * cos(angle) - (point.y - centre.y) * sin(angle));
	ret.y = (centre.y + (point.x - centre.x) * sin(angle) + (point.y - centre.y) * cos(angle));
	return ret;
}

double normalizeAngle(double angle) {
	if (angle < 0) {
		return angle + 2 * (2 * acos(0.0));  // Shift negative angles into [0, 2pi]
	}
	return angle;
}

//Creates a new Vector2
Vector2 newVector2(float x, float y) {
	Vector2 ret = {};
	ret.x = x;
	ret.y = y;
	return ret;
}

//Creates a new Vector3
Vector3 newVector3(float x, float y, float z) {
	Vector3 ret = {};
	ret.x = x;
	ret.y = y;
	ret.z = z;
	return ret;
}