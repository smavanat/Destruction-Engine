#pragma once
#include <SDL3/SDL.h>
#include<SDL3_image/SDL_image.h>
#include<stdio.h>
#include<iostream>
#include "Maths.h"
//Texture class. This contains functionality for creating textures that are loaded from files or from dynamically
//created pixel buffers. It has no control over manipulating its own pixels. This is handled externally.
class Texture {
	public:
		Texture();

		Texture(int x, int y);

		Texture(int x, int y, int w, int h, Uint32* pixels, SDL_Renderer* gRenderer, double d);

		~Texture();

		bool loadFromFile(std::string path, SDL_Renderer* gRenderer);

		bool loadPixelsFromFile(std::string path);

		bool loadFromPixels(SDL_Renderer* gRenderer);

		bool isAltered();

		bool clickedOnTransparent(int x, int y);

		void free();

		//void setOrigin(int x, int y);
		
		void setCentre(float x, float y);

		void setAngle(double d);

		void render(SDL_Renderer* gRenderer);

		void render(SDL_Renderer* gRenderer, SDL_FRect* clip, double angle, SDL_FPoint* centre, SDL_FlipMode flip);

		void markAsAltered();

		void resetSplittingFlag();

		int getWidth();
		int getHeight();
		double getAngle();

		Uint32* getPixels32();
		Uint32 getPitch32();
		Uint32 mapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		SDL_PixelFormat getPixelFormat();
		Vector2 getOrigin();
		Vector2 getCentre();

	private:
		SDL_Texture* texture;

		SDL_Surface* surfacePixels;

		Vector2 centre = {};

		int width;
		int height;

		double angle;

		bool needsSplitting;
};

