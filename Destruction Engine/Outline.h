#pragma once
#include<SDL3/SDL.h>
#include<stdio.h>
#include<vector>
#include<box2d/box2d.h>
#include<PolyPartition/polypartition.h>
#include "BasicComponents.h"
const Uint32 NO_PIXEL_COLOUR = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_ARGB8888), NULL, 0xff, 0xff, 0xff, 0x00);

void erasePixels(Sprite &s, Transform& t, SDL_Renderer* gRenderer, int scale, int x, int y);

void erasePixelsRectangle(Sprite &s, Transform& t, SDL_Renderer* gRenderer, int halfX, int halfY, float rot, int x, int y);

bool isAtTopEdge(int pixelPosition, int arrayWidth);

bool isAtBottomEdge(int pixelPosition, int arrayWidth, int arrayLength);

bool isAtLeftEdge(int pixelPosition, int arrayWidth);

bool isAtRightEdge(int pixelPosition, int arrayWidth);

int* getNeighbours(int pixelPosition, int arrayWidth, int arrayLength);

void cleanup(Uint32* pixels, std::vector<int> indexes);

std::vector<int> bfs(int index, int arrayWidth, int arrayLength, Uint32* pixels, int* visitedTracker);

std::pair<Sprite, Transform> constructNewPixelBuffer(std::vector<int> indexes, Uint32* pixels, int arrayWidth, Sprite& s, Transform& t, SDL_Renderer* gRenderer);

std::vector<std::pair<Sprite, Transform>> splitTextureAtEdge(Sprite& s, Transform& t, SDL_Renderer* gRenderer);

int getStartingPixel(Uint32* pixels, int arrayLength);

int getCurrentSquare(int startIndex, int textureWidth, int textureLength, const Uint32* pixels);

std::vector<int> marchingSquares(Sprite s);

int* convertIndexToCoords(int index, int arrayWidth);

float lineDist(int point, int startPoint, int endPoint, int arrayWidth);

int findFurthest(std::vector<int> allPoints, int a, int b, int epsilon, int arrayWidth);

void rdp(int startIndex, int endIndex, int epsilon, int arrayWidth, std::vector<int> allPoints, std::vector<int>& rdpPoints);

b2BodyId createTexturePolygon(std::vector<int> rdpPoints, int arrayWidth, b2WorldId worldId, b2BodyType type, Transform& t);
