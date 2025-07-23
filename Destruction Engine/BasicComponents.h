#pragma once
#include "Component.h"
#include<SDL3_image/SDL_image.h>
#include<iostream>
#include "box2d/base.h"
#include "Maths.h"
#include <vector>

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
    Vector2 centre = {};
    int width = 0;
    int height = 0;
    double angle = 0.0;
    bool needsSplitting = false;

    Sprite() = default;

    Sprite(
        SDL_Texture* texture,
        SDL_Surface* surfacePixels,
        const Vector2& centre,
        int width,
        int height,
        double angle,
        bool needsSplitting)
        : texture(texture),
        surfacePixels(surfacePixels),
        centre(centre),
        width(width),
        height(height),
        angle(angle),
        needsSplitting(needsSplitting)
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

//Determines the state of a partially walkable tile:
struct PartiallyWalkable : public Component<PartiallyWalkable> {
    int status; //0 -> walkable, 1 -> blocked 2 -> partial
    int subcells[16];

    PartiallyWalkable() = default;

    PartiallyWalkable(int s, int sArr[16]) : status(s) {
        std::copy(sArr, sArr + 16, subcells);
    }
};

//Tile type -> represents what bit of a spritesheet to use for a tileset
struct TileType : public Component<TileType> {
    int type;

    TileType() = default;

    TileType(int t) : type(t) {}
};

//Holds the start and end positions to find a path between, and the path returned
struct Pathfinding : public Component<Pathfinding> {
    Vector2 startPos;
    Vector2 endPos;
    std::vector<Vector2> path;

    Pathfinding() = default;

    Pathfinding(Vector2 s, Vector2 e) : startPos(s), endPos(e), path(std::vector<Vector2>()) {};
};

Vector2 getOrigin(Sprite s);

void free(Sprite &s);

bool loadFromPixels(Sprite &s, SDL_Renderer* gRenderer);

bool loadPixelsFromFile(Sprite &s, std::string path);

bool loadFromFile(Sprite &s, std::string path, SDL_Renderer* gRenderer);

//Basic sprite creation (usually used when loading from a file)
Sprite createSprite(int x, int y);

//Controlled sprite creation (when making an object after destruction)
Sprite createSprite(int x, int y, int w, int h, Uint32* pixels, SDL_Renderer* gRenderer, double d);

bool clickedOnTransparent(Sprite s, int x, int y);

Uint32* getPixels32(Sprite s);

Uint32 getPitch32(Sprite s);

Uint32 mapRGBA(Sprite &s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

void renderBasic(Sprite s, SDL_Renderer* gRenderer);

//When need a rotateable texture
void render(Sprite s, SDL_Renderer* gRenderer);

SDL_PixelFormat getPixelFormat(Sprite s);