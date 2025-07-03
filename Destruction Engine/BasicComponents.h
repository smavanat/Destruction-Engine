#pragma once
#include "Maths.h"
#include "Component.h"
#include<iostream>
#include<SDL3_image/SDL_image.h>
#include "box2d/base.h"
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

//Sprite-related functions (modified versions of the original texture functions):
#pragma region SpriteFunctions
inline Vector2 s_getOrigin(Sprite s) {
    float originX = s.centre.x - (s.width / 2);
    float originY = s.centre.y - (s.height / 2);
    return newVector2(originX, originY);
}

inline void s_free(Sprite &s) {
    if (s.texture != NULL) {
        SDL_DestroyTexture(s.texture);
        s.texture = NULL;
        s.width = 0;
        s.height = 0;
    }

    if (s.surfacePixels != NULL) {
        SDL_DestroySurface(s.surfacePixels);
        s.surfacePixels = NULL;
    }
}

inline bool s_loadFromPixels(Sprite &s, SDL_Renderer* gRenderer) {
    if (s.surfacePixels == NULL) {
        printf("No pixels loaded!");
    }
    else {
        s.texture = SDL_CreateTextureFromSurface(gRenderer, s.surfacePixels);
        SDL_SetTextureBlendMode(s.texture, SDL_BLENDMODE_BLEND);
        if (s.texture == NULL)
        {
            printf("Unable to create texture from loaded pixels! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            s.width = s.surfacePixels->w;
            s.height = s.surfacePixels->h;
        }

    }
    return s.texture != NULL;
}

inline bool s_loadPixelsFromFile(Sprite &s, std::string path) {
    s_free(s);

    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError());
    }
    else {
        s.surfacePixels = SDL_ConvertSurface(loadedSurface, SDL_PIXELFORMAT_ARGB8888);
        if (s.surfacePixels == NULL) {
            printf("Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError());
        }
        else {
            s.width = s.surfacePixels->w;
            s.height = s.surfacePixels->h;
        }
        SDL_DestroySurface(loadedSurface);
    }
    return s.surfacePixels != NULL;
}

inline bool s_loadFromFile(Sprite &s, std::string path, SDL_Renderer* gRenderer) {
    if (!s_loadPixelsFromFile(s, path)) {
        printf("Failed to load pixels for %s!\n", path.c_str());
    }
    else {
        if (!s_loadFromPixels(s, gRenderer))
        {
            printf("Failed to texture from pixels from %s!\n", path.c_str());
        }
    }

    return s.texture != NULL;
}

//Basic sprite creation (usually used when loading from a file)
inline Sprite s_createSprite(int x, int y) {
    return Sprite(nullptr, nullptr, newVector2(x, y), 0, 0, 0.0, false);
}

//Controlled sprite creation (when making an object after destruction)
inline Sprite s_createSprite(int x, int y, int w, int h, Uint32* pixels, SDL_Renderer* gRenderer, double d) {
    Sprite s = Sprite(NULL, SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ARGB8888, pixels, w * 4), newVector2(x, y), w, h, d, false);
    SDL_SetSurfaceBlendMode(s.surfacePixels, SDL_BLENDMODE_BLEND);
    s_loadFromPixels(s, gRenderer);
    return s;
}

inline bool s_clickedOnTransparent(Sprite s, int x, int y) {
    // Adjust for the origin of the texture
    x -= s_getOrigin(s).x;
    y -= s_getOrigin(s).y;

    // Get the pixel format for accessing the surface
    Uint8 alpha;
    // Get the pixel data at (x, y) and retrieve RGBA values
    SDL_GetRGBA(((Uint32*)s.surfacePixels->pixels)[y * s.surfacePixels->w + x],
        SDL_GetPixelFormatDetails(s.surfacePixels->format), NULL, NULL, NULL, NULL, &alpha);

    // Check if the pixel is fully transparent (alpha == 0)
    if (alpha == 0) {
        return true;
    }
    return false;
}

inline Uint32* s_getPixels32(Sprite s) {
    Uint32* pixels = NULL;

    if (s.surfacePixels != NULL) {
        pixels = static_cast<Uint32*>(s.surfacePixels->pixels);
    }

    return pixels;
}

inline Uint32 s_getPitch32(Sprite s) {
    Uint32 pitch = 0;

    if (s.surfacePixels != NULL)
    {
        pitch = s.surfacePixels->pitch / 4;
    }

    return pitch;
}

inline Uint32 s_mapRGBA(Sprite &s, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint32 pixel = 0;

    if (s.surfacePixels != NULL)
    {
        pixel = SDL_MapRGBA(SDL_GetPixelFormatDetails(s.surfacePixels->format), NULL, r, g, b, a);
    }

    return pixel;
}

inline void s_renderBasic(Sprite s, SDL_Renderer* gRenderer) {
    SDL_FRect renderQuad = { s_getOrigin(s).x, s_getOrigin(s).y, s.width, s.height };
    SDL_RenderTexture(gRenderer, s.texture, NULL, &renderQuad);
}

//When need a rotateable texture
inline void s_render(Sprite s, SDL_Renderer* gRenderer) {
    SDL_FRect renderQuad = { s_getOrigin(s).x, s_getOrigin(s).y, s.width, s.height };

    SDL_RenderTextureRotated(gRenderer, s.texture, NULL, &renderQuad, s.angle, NULL, SDL_FLIP_NONE);
}

inline SDL_PixelFormat s_getPixelFormat(Sprite s) {
    return s.surfacePixels->format;
}
#pragma endregion

struct Collider : public Component<Collider> {
	b2BodyId colliderId;

    Collider() = default;

    Collider(b2BodyId colliderId) : colliderId(colliderId) {}
};

struct Tag : public Component<Tag> {
    int tagId;

    Tag() = default;

    Tag(int tagId) : tagId(tagId) {}
};

struct Walkable : public Component<Walkable> {
    int walkStatus;

    Walkable() = default;

    Walkable(int w) : walkStatus(w) {};
};

struct TileType : public Component<TileType> {
    int type;

    TileType() = default;

    TileType(int t) : type(t) {}
};