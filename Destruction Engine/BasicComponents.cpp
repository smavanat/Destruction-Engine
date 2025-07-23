#include "BasicComponents.h"

//Sprite-related functions (modified versions of the original texture functions):
#pragma region SpriteFunctions
Vector2 getOrigin(Sprite s) {
    float originX = s.centre.x - (s.width / 2);
    float originY = s.centre.y - (s.height / 2);
    return Vector2(originX, originY);
}

void free(Sprite &s) {
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

bool loadFromPixels(Sprite &s, SDL_Renderer* gRenderer) {
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

bool loadPixelsFromFile(Sprite &s, std::string path) {
    free(s);

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

bool loadFromFile(Sprite &s, std::string path, SDL_Renderer* gRenderer) {
    if (!loadPixelsFromFile(s, path)) {
        printf("Failed to load pixels for %s!\n", path.c_str());
    }
    else {
        if (!loadFromPixels(s, gRenderer))
        {
            printf("Failed to load texture from pixels from %s!\n", path.c_str());
        }
    }

    return s.texture != NULL;
}

//Basic sprite creation (usually used when loading from a file)
Sprite createSprite(int x, int y) {
    return Sprite(nullptr, nullptr, Vector2(x, y), 0, 0, 0.0, false);
}

//Controlled sprite creation (when making an object after destruction)
Sprite createSprite(int x, int y, int w, int h, Uint32* pixels, SDL_Renderer* gRenderer, double d) {
    Sprite s = Sprite(NULL, SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ARGB8888, pixels, w * 4), Vector2(x, y), w, h, d, false);
    SDL_SetSurfaceBlendMode(s.surfacePixels, SDL_BLENDMODE_BLEND);
    loadFromPixels(s, gRenderer);
    return s;
}

bool clickedOnTransparent(Sprite s, int x, int y) {
    // Adjust for the origin of the texture
    x -= getOrigin(s).x;
    y -= getOrigin(s).y;

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

Uint32* getPixels32(Sprite s) {
    Uint32* pixels = NULL;

    if (s.surfacePixels != NULL) {
        pixels = static_cast<Uint32*>(s.surfacePixels->pixels);
    }

    return pixels;
}

Uint32 getPitch32(Sprite s) {
    Uint32 pitch = 0;

    if (s.surfacePixels != NULL)
    {
        pitch = s.surfacePixels->pitch / 4;
    }

    return pitch;
}

Uint32 mapRGBA(Sprite &s, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint32 pixel = 0;

    if (s.surfacePixels != NULL)
    {
        pixel = SDL_MapRGBA(SDL_GetPixelFormatDetails(s.surfacePixels->format), NULL, r, g, b, a);
    }

    return pixel;
}

void renderBasic(Sprite s, SDL_Renderer* gRenderer) {
    SDL_FRect renderQuad = { getOrigin(s).x, getOrigin(s).y, s.width, s.height };
    SDL_RenderTexture(gRenderer, s.texture, NULL, &renderQuad);
}

//When need a rotateable texture
void render(Sprite s, SDL_Renderer* gRenderer) {
    SDL_FRect renderQuad = { getOrigin(s).x, getOrigin(s).y, s.width, s.height };

    SDL_RenderTextureRotated(gRenderer, s.texture, NULL, &renderQuad, s.angle, NULL, SDL_FLIP_NONE);
}

SDL_PixelFormat getPixelFormat(Sprite s) {
    return s.surfacePixels->format;
}

//Creates a copy of a sprite at a new position
Sprite duplicateSprite(int x, int y, double d, Sprite* original, SDL_Renderer* gRenderer, SDL_FRect* srcRect) {
    Uint32* newPixelArray = (Uint32*)malloc(srcRect->h * srcRect->w * sizeof(Uint32));
    for (int row = 0; row < srcRect->h; ++row) {
        int srcIndex = (srcRect->y + row) * original->width + srcRect->x;
        int dstIndex = row * srcRect->w;

        memcpy(&newPixelArray[dstIndex], &original->surfacePixels[srcIndex], srcRect->w * sizeof(Uint32));
    }

    return createSprite(x, y, srcRect->w, srcRect->h, newPixelArray, gRenderer, d);
}
#pragma endregion