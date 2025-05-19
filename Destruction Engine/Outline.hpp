#pragma once
#include<SDL3/SDL.h>
#include<stdio.h>
#include<iostream>
#include<vector>
#include<box2d/box2d.h>
#include<PolyPartition/polypartition.h>
#include "Texture.hpp"
const float metresToPixels = 50.0f;
const float pixelsToMetres = 1.0f / metresToPixels;
const Uint32 NO_PIXEL_COLOUR = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_ARGB8888), NULL, 0xff, 0xff, 0xff, 0x00);

void erasePixels(Texture* texture, SDL_Renderer* gRenderer, int scale, int x, int y);

bool isAtTopEdge(int pixelPosition, int arrayWidth);

bool isAtBottomEdge(int pixelPosition, int arrayWidth, int arrayLength);

bool isAtLeftEdge(int pixelPosition, int arrayWidth);

bool isAtRightEdge(int pixelPosition, int arrayWidth);

int* getNeighbours(int pixelPosition, int arrayWidth, int arrayLength);

//bool findColoursOfNeighbours(int pixelPosition, int arrayWidth, int arrayLength, Uint32* bufferArray);

void cleanup(Uint32* pixels, std::vector<int> indexes);

//std::vector<int> contourFinder(Uint32* pixels, Uint32 noPixelColour)

std::vector<int> bfs(int index, int arrayWidth, int arrayLength, Uint32* pixels, int* visitedTracker);

Texture* constructNewPixelBuffer(std::vector<int> indexes, Uint32* pixels, int arrayWidth, Texture* texture, SDL_Renderer* gRenderer);

std::vector<Texture*> splitTextureAtEdge(Texture* texture, SDL_Renderer* gRenderer);

int getStartingPixel(Uint32* pixels, int arrayLength);

int getCurrentSquare(int startIndex, int textureWidth, int textureLength, const Uint32* pixels);

std::vector<int> marchingSquares(Texture* texture);

int* convertIndexToCoords(int index, int arrayWidth);

float lineDist(int point, int startPoint, int endPoint, int arrayWidth);

int findFurthest(std::vector<int> allPoints, int a, int b, int epsilon, int arrayWidth);

void rdp(int startIndex, int endIndex, int epsilon, int arrayWidth, std::vector<int> allPoints, std::vector<int>& rdpPoints);

b2Vec2* getVec2Array(std::vector<int> rdpPoints, int arrayWidth);

b2Vec2* convertToVec2(TPPLPoint* polyPoints, int numPoints);

void rotateTranslate(b2Vec2& vector, float angle);

b2BodyId createTexturePolygon(std::vector<int> rdpPoints, int arrayWidth, b2WorldId worldId, Texture* texture);