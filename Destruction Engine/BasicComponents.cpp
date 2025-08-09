#include "BasicComponents.h"

//Sprite-related functions:
#pragma region SpriteFunctions
Vector2 getOrigin(Sprite& s, Transform& t) {
    float originX = t.position.x - (s.surfacePixels->w / 2);
    float originY = t.position.y - (s.surfacePixels->h / 2);
    return (Vector2){originX, originY};
}

Vector2 getOrigin(TileSprite& s, Transform& t) {
    float originX = t.position.x - (s.srcRect->w / 2);
    float originY = t.position.y - (s.srcRect->h / 2);
    return (Vector2){originX, originY};
}

void free(Sprite &s) {
    if (s.texture != NULL) {
        SDL_DestroyTexture(s.texture);
        s.texture = NULL;
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

//Controlled sprite creation (when making an object after destruction)
Sprite createSprite(int w, int h, Uint32* pixels, SDL_Renderer* gRenderer) {
    Sprite s = Sprite(NULL, SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_ARGB8888, pixels, w * 4), false);
    SDL_SetSurfaceBlendMode(s.surfacePixels, SDL_BLENDMODE_BLEND);
    loadFromPixels(s, gRenderer);
    return s;
}

bool clickedOnTransparent(Sprite& s, Transform& t, int x, int y) {
    // Adjust for the origin of the texture
    x -= getOrigin(s, t).x;
    y -= getOrigin(s, t).y;

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

Uint32* getPixels32(Sprite& s) {
    Uint32* pixels = NULL;

    if (s.surfacePixels != NULL) {
        pixels = static_cast<Uint32*>(s.surfacePixels->pixels);
    }

    return pixels;
}

Uint32 getPitch32(Sprite& s) {
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

void renderBasic(Sprite& s, Transform& t, SDL_Renderer* gRenderer) {
    SDL_FRect renderQuad = { getOrigin(s, t).x, getOrigin(s, t).y, s.surfacePixels->w, s.surfacePixels->h };
    SDL_RenderTexture(gRenderer, s.texture, NULL, &renderQuad);
}

//When need a rotateable texture
void render(Sprite& s, Transform& t, SDL_Renderer* gRenderer) {
    SDL_FRect renderQuad = { getOrigin(s, t).x, getOrigin(s, t).y, s.surfacePixels->w, s.surfacePixels->h };

    SDL_RenderTextureRotated(gRenderer, s.texture, NULL, &renderQuad, t.rotation, NULL, SDL_FLIP_NONE);
}

void renderPart(TileSprite& s, Transform& t, SDL_Renderer* gRenderer) {
    SDL_FRect renderQuad = { getOrigin(s, t).x, getOrigin(s, t).y, s.srcRect->w, s.srcRect->h };
    SDL_RenderTexture(gRenderer, s.srcTex, s.srcRect, &renderQuad);
}

SDL_PixelFormat getPixelFormat(Sprite s) {
    return s.surfacePixels->format;
}

//Creates a copy of a sprite at a new position
Sprite duplicateSprite(Sprite* original, SDL_Renderer* gRenderer, SDL_FRect* srcRect) {
    int width = (int)srcRect->w;
    int height = (int)srcRect->h;

    Uint32* newPixelArray = (Uint32*)malloc(width * height * sizeof(Uint32));
    for (int row = 0; row < height; ++row) {
        Uint32* pixels = (Uint32*)original->surfacePixels->pixels;
        int pitch = original->surfacePixels->pitch / sizeof(Uint32);
        int srcIndex = (srcRect->y + row) * pitch + srcRect->x;
        //int srcIndex = (srcRect->y + row) * original->surfacePixels->w + srcRect->x;
        int dstIndex = row * width;

        memcpy(&newPixelArray[dstIndex], &pixels[srcIndex], width * sizeof(Uint32));
    }
    return createSprite(width, height, newPixelArray, gRenderer);
}
#pragma endregion

//Collider related functions
#pragma region Collider
b2BodyId createCircleCollider(Vector2 center, float radius, b2WorldId worldId, b2BodyType type) {
    //Default shape initialisation code
	b2BodyDef retBodyDef = b2DefaultBodyDef();
	retBodyDef.type = type;
    retBodyDef.position = {center.x * pixelsToMetres, center.y * pixelsToMetres};
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Circle creation
    b2Circle circle = {{0.0f, 0.0f}, radius*pixelsToMetres};
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreateCircleShape(retId, &sd, &circle);
	
    return retId;
}

b2BodyId createBoxCollider(Vector2 center, int width, int height, float rotation, b2WorldId worldId, b2BodyType type) {
    //Default shape initialisation code
	b2BodyDef retBodyDef = b2DefaultBodyDef();
	retBodyDef.type = type;
    retBodyDef.position = {center.x * pixelsToMetres, center.y * pixelsToMetres};
    retBodyDef.rotation = { (float)cos(rotation * DEGREES_TO_RADIANS), (float)sin(rotation * DEGREES_TO_RADIANS) };
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Box creation
    b2Polygon box = b2MakeBox((width/2)*pixelsToMetres, (height/2)*pixelsToMetres);
    b2ShapeDef sd = b2DefaultShapeDef();
    b2CreatePolygonShape(retId, &sd, &box);
	
    return retId;
}

b2BodyId createCapsuleCollider(Vector2 center1, Vector2 center2, float radius, float rotation, b2WorldId worldId, b2BodyType type) {
    //Calculate actual center of the capsule
    Vector2 center = (Vector2){(center2.x - center1.x)/2.0f, (center2.y - center1.y)/2.0f};

    //Default shape initialisation code
	b2BodyDef retBodyDef = b2DefaultBodyDef();
	retBodyDef.type = type;
    retBodyDef.position = {center.x * pixelsToMetres, center.y * pixelsToMetres};
    retBodyDef.rotation = { (float)cos(rotation * DEGREES_TO_RADIANS), (float)sin(rotation * DEGREES_TO_RADIANS) };
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);

    //Capsule creation
    b2Capsule capsule = {{center1.x*pixelsToMetres, center1.y*pixelsToMetres}, {center2.x*pixelsToMetres, center2.y*pixelsToMetres}, radius*pixelsToMetres};
	b2ShapeDef sd = b2DefaultShapeDef();
    b2CreateCapsuleShape(retId, &sd, &capsule);

    return retId;
}

//Needs to exist for now before polypartition rework
Vector2* convertToVec2(TPPLPoint* polyPoints, int numPoints) {
    Vector2* points = new Vector2[numPoints];
    for (int i = 0; i < numPoints; i++) {
        points[i].x = polyPoints[i].x;
        points[i].y = polyPoints[i].y;
    }
    return points;
}

//Rotates a point a given angle
Vector2 rotateTranslate(Vector2& vector, float angle) {
    Vector2 tmp;
    tmp.x = vector.x * cos(angle) - vector.y * sin(angle);
    tmp.y = vector.x * sin(angle) + vector.y * cos(angle);
    return tmp;
}

//Finds the center of a shape assuming that it has been partitioned into triangles
TPPLPoint ComputeWeightedCompoundCentroid(TPPLPolyList &shapes) {
    TPPLPoint weightedCentroid = {0.0f, 0.0f};
    float totalArea = 0.0f;

    for (TPPLPolyList::iterator it = shapes.begin(); it != shapes.end(); ++it) {
        //Getting the vertices
        TPPLPoint A = it->GetPoint(0);
        TPPLPoint B = it->GetPoint(1);
        TPPLPoint C = it->GetPoint(2);

        //Getting the centroid
        TPPLPoint centroid = (A + B + C) / 3.0f;

        //Getting the area
        float area = 0.5f * fabs(A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));

        //Accumulate weighted centroid sum:
        weightedCentroid.x += centroid.x * area;
        weightedCentroid.y += centroid.y * area;
        totalArea += area;
    }

    if (totalArea != 0) {
        weightedCentroid.x /= totalArea;
        weightedCentroid.y /= totalArea;
    }

    return weightedCentroid;
}

//Calculates the unweighted centroid of a shape by just finding the center of its bounding box
TPPLPoint ComputeCompoundCentroid(TPPLPolyList &shapes) {
    //Get the bounding box:
    double minX = 0.0, minY = 0.0, maxX = 0.0, maxY = 0.0;

    for (TPPLPolyList::iterator it = shapes.begin(); it != shapes.end(); ++it) {
        for (int i = 0; i < it->GetNumPoints(); i++) {
            if(it->GetPoint(i).x < minX) minX = it->GetPoint(i).x;
            if(it->GetPoint(i).y < minY) minY = it->GetPoint(i).y;
            if(it->GetPoint(i).x > maxX) maxX = it->GetPoint(i).x;
            if(it->GetPoint(i).y > maxY) maxY = it->GetPoint(i).y;
        }
    }
    return (TPPLPoint){minX + (maxX-minX)/2.0f, minY + (maxY-minY)/2.0f};
}

//Centers a shape around its unweighted centroid
void CenterCompundShape(TPPLPolyList &shapes) {
    TPPLPoint compoundCentroid = ComputeCompoundCentroid(shapes);

    for (TPPLPolyList::iterator it = shapes.begin(); it != shapes.end(); ++it) {
        for (int i = 0; i < it->GetNumPoints(); i++) {
            it->GetPoint(i).x -= compoundCentroid.x;
            it->GetPoint(i).y -= compoundCentroid.y;
        }
    }
}
//Creates a texture polygon by using pure triangulation, and then moves the origin so that it is in the centre of the 
//shape rather than at the top-left corner.
b2BodyId createPolygonCollider(Vector2* points, int pointsSize, Vector2 center, float rotation, b2WorldId worldId, b2BodyType type) {
    //Default polygon initialisation
    b2BodyDef retBodyDef = b2DefaultBodyDef();
	retBodyDef.type = type;
    retBodyDef.position = {center.x * pixelsToMetres, center.y * pixelsToMetres};
    retBodyDef.rotation = { (float)cos(rotation * DEGREES_TO_RADIANS), (float)sin(rotation * DEGREES_TO_RADIANS) };
    b2BodyId retId = b2CreateBody(worldId, &retBodyDef);
	
    //I am going to partition the polygon regardless of whether or not the number of vertices is less than 8, because
    //Box2D does some very aggressive oversimplification of the shape outline which I'm not a fan of.
    //It is better to just put in triangles so it can't mess things up. I am going to use triangulation instead of 
    //partitioning to make sure Box2D keeps all of the details, as in higher-vertex convex shapes there is a change
    //simplification could occur, which I want to avoid. This also helps to make sure that the vertices of the sub-polygons is
    //standardised.

    //Creating the polygon for Polypartition
    TPPLPoly* poly = new TPPLPoly();
    poly->Init(pointsSize);
    TPPLPolyList polyList;

    for (int i = 0; i < pointsSize; i++) {
        (*poly)[i].x = points[i].x;
        (*poly)[i].y = points[i].y;
    }

    //Need to set it to be oriented Counter-Clockwise otherwise the triangulation algorithm fails.
    poly->SetOrientation(TPPL_ORIENTATION_CCW); //This method does not actually check the order of each vertex. Need to change it so it sorts the points properly.
    TPPLPartition test = TPPLPartition(); 
    int result = test.Triangulate_OPT(poly, &polyList); //Traingulate the polygon shape

    //Trying to center the polygon:
    CenterCompundShape(polyList);

    //Adding the polygons to the collider, or printing an error message if something goes wrong.
    for (TPPLPolyList::iterator it = polyList.begin(); it != polyList.end(); ++it) {
        b2Hull hull = b2ComputeHull(convertToVec2(it->GetPoints(), it->GetNumPoints()), it->GetNumPoints());
        if (hull.count == 0) {
            printf("Something odd has occured when generating a hull from a polyList\n");
        }
        else {
            b2Polygon testagon = b2MakePolygon(&hull, 0.0f);
            b2ShapeDef testshapeDef = b2DefaultShapeDef();
            b2ShapeId testShapeId = b2CreatePolygonShape(retId, &testshapeDef, &testagon);
            b2Shape_SetFriction(testShapeId, 0.3);
        }
    }

    return retId;
}
#pragma endregion
