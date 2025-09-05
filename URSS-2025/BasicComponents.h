#pragma once
#include "Component.h"
#include<SDL3_image/SDL_image.h>
#include<SDL3/SDL.h>
#include<iostream>
#include<PolyPartition/polypartition.h>
#include "box2d/base.h"
#include "Maths.h"
//#include "Outline.h"
#include <vector>
const float metresToPixels = 50.0f;
const float pixelsToMetres = 1.0f / metresToPixels;
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

enum ColliderType {
	BOX,
	CIRCLE,
	CAPSULE,
	POLYGON,
	NONE
};

//Holds a box2D b2BodyId
struct Collider : public Component<Collider> {
	b2BodyId colliderId;
	ColliderType type;	

    Collider() = default;

    Collider(b2BodyId colliderId, ColliderType type) : colliderId(colliderId), type(type) {}
};

//Tag component to seperate identities that would otherwise be identical into groups
struct Tag : public Component<Tag> {
    int tagId;

    Tag() = default;

    Tag(int tagId) : tagId(tagId) {}
};

struct TileRect : public Component<TileRect> {
    SDL_FRect* dimensions;

    TileRect() = default;

    TileRect(SDL_FRect* s) : dimensions(s) {};
};

//Holds the start and end positions to find a path between, and the path returned
struct Pathfinding : public Component<Pathfinding> {
    std::vector<Vector2> path; //Change this to a C-style array (Vector2*)
    Vector2 startPos;
    Vector2 endPos;
    int size;

    Pathfinding() = default;

    Pathfinding(Vector2 st, Vector2 e, int s) : startPos(st), endPos(e), path(std::vector<Vector2>()), size(s) {};
};

//A struct that holds if a destructible entity is a part of the terrain or not
struct Terrain : public Component<Terrain> {
    bool isTerrain;

    Terrain() = default;

    Terrain(bool t) : isTerrain(t) {};
};

//For handling world state in the demo
enum DEMO_STATE {
    FIRST,
    SECOND,
    THIRD
};

struct Button : public Component<Button> {
    SDL_FRect* area;
    DEMO_STATE state;

    Button() = default;

    Button(SDL_FRect* a, DEMO_STATE s) : area(a), state(s) {};
};

//(Tile)Sprite functions

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
void render(Sprite& s, Transform& t, SDL_Renderer* gRenderer);
//When rendering the sprite from another texture
void renderPart(TileSprite& s, Transform& t, SDL_Renderer* gRenderer);
SDL_PixelFormat getPixelFormat(Sprite s);
Sprite duplicateSprite(Sprite* original, SDL_Renderer* gRenderer, SDL_FRect* srcRect);

//Collider functions

b2BodyId createCircleCollider(Vector2 center, float radius, b2WorldId worldId, b2BodyType type);
b2BodyId createBoxCollider(Vector2 center, int width, int height, float rotation, b2WorldId worldId, b2BodyType type);
b2BodyId createCapsuleCollider(Vector2 center1, Vector2 center2, float rotation, float radius, b2WorldId worldId, b2BodyType type);
b2BodyId createPolygonCollider(Vector2* points, int pointsSize, Vector2 center, float rotation, b2WorldId worldId, b2BodyType type);
Vector2 rotateTranslate(Vector2& vector, float angle);
