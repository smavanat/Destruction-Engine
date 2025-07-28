#pragma once
#include "Component.h"
#include<SDL3_image/SDL_image.h>
#include<SDL3/SDL.h>
#include<iostream>
#include "box2d/base.h"
#include "Maths.h"
#include <vector>

//Need to forward declare these above the other functions/structs so that the default value for the Sprite constructor is held
struct Sprite;
struct Transform;
//When need a rotateable texture. 
void render(Sprite& s, Transform& t, SDL_Renderer* gRenderer);


//Holds the position and rotation of an entity
struct Transform : public Component<Transform> {
	Vector2 position;
	double rotation;

    Transform() = default;

    Transform(
        Vector2 position,
        double rotation
    ) :
    position(position),
    rotation(rotation)
    {}
};

//Holds sprite texture data
struct Sprite : public Component<Sprite> {
    SDL_Texture* texture = nullptr;
    SDL_Surface* surfacePixels = nullptr; 
    bool needsSplitting = false; //How can we get rid of this bool to avoid wasting space

    Sprite() = default;

    Sprite(
        SDL_Texture* texture,
        SDL_Surface* surfacePixels,
        bool needsSplitting)
        : texture(texture),
        surfacePixels(surfacePixels),
        needsSplitting(needsSplitting)
    {}
};

struct TileSprite : public Component<TileSprite> {
    SDL_Texture* srcTex = nullptr;
    SDL_FRect* srcRect = nullptr;

    TileSprite() = default;

    TileSprite(
        SDL_Texture* tex,
        SDL_FRect* src)
        : srcTex(tex),
        srcRect(src)
    {}
};

//Holds a box2D b2BodyId
struct Collider : public Component<Collider> {
	b2BodyId colliderId;

    Collider() = default;

    Collider(b2BodyId colliderId) : colliderId(colliderId) {}
};

//Tag component to seperate identities that would otherwise be identical into groups
struct Tag : public Component<Tag> {
    int tagId;

    Tag() = default;

    Tag(int tagId) : tagId(tagId) {}
};

//Whether a tile is walkable or not
struct Walkable : public Component<Walkable> {
    int walkStatus;

    Walkable() = default;

    Walkable(int w) : walkStatus(w) {};
};

//Holds the start and end positions to find a path between, and the path returned
struct Pathfinding : public Component<Pathfinding> {
    Vector2 startPos;
    Vector2 endPos;
    std::vector<Vector2> path;

    Pathfinding() = default;

    Pathfinding(Vector2 s, Vector2 e) : startPos(s), endPos(e), path(std::vector<Vector2>()) {};
};

Vector2 getOrigin(Sprite& s, Transform& t);

void free(Sprite &s);

bool loadFromPixels(Sprite &s, SDL_Renderer* gRenderer);

bool loadPixelsFromFile(Sprite &s, std::string path);

bool loadFromFile(Sprite &s, std::string path, SDL_Renderer* gRenderer);

//Controlled sprite creation (when making an object after destruction)
Sprite createSprite(int w, int h, Uint32* pixels, SDL_Renderer* gRenderer);

bool clickedOnTransparent(Sprite& s, Transform& t, int x, int y);

Uint32* getPixels32(Sprite& s);

Uint32 getPitch32(Sprite& s);

Uint32 mapRGBA(Sprite &s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

void renderBasic(Sprite& s, Transform& t, SDL_Renderer* gRenderer);

//When rendering the sprite from another texture
void renderPart(TileSprite& s, Transform& t, SDL_Renderer* gRenderer);

SDL_PixelFormat getPixelFormat(Sprite s);

Sprite duplicateSprite(Sprite* original, SDL_Renderer* gRenderer, SDL_FRect* srcRect);