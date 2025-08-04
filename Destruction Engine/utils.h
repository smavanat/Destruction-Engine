#pragma once
#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include "Maths.h"

//#define bool unsigned char
#ifdef __cplusplus
extern "C" {
#endif
// A simple implmenentation of a multimap on entities, where instead of storing pointers, we
// simply store indecies into component arrays. This could break when objects are deleted from the 
// array, so need to be careful.
typedef struct {
	uint32_t* keys;
	uint32_t** values;
} multiMap;

typedef struct {

} memoryArena;

int drawCircle(SDL_Renderer* gRenderer, Vector2 center, float radius);

#ifdef __cplusplus
}
#endif

#endif