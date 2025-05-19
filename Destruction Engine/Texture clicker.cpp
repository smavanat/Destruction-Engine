#include "Outline.hpp"
#include<SDL3_image/SDL_image.h>

//TODO: Figure out how do deal with small shapes. Colliders are not generated for them, but they are still there.
//		Maybe just erase them? Or put a default small collider around them.

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

////Some global variables
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
Texture testTexture = Texture(320.0f, 240.0f);
b2WorldDef worldDef;
b2WorldId worldId;

//For holding our textures. Needs to be a pointer vector because otherwise will get error because the texture objects 
//are being copied over to a new location and the originals deleted.
//This means that the old pointers for surfacePixels etc. are being invalidated, hence the memory write errors.
std::vector<Texture*> textures;

std::vector<b2BodyId> colliders;	
std::vector<b2BodyId> statics;

int scale = 5;

bool init();
bool loadMedia();

//Necessary to check which colliders to remove from vector since c++ does not generate default comparators for structs.
bool operator ==(const b2BodyId& lhs, const b2BodyId& rhs) {
	if (lhs.index1 == rhs.index1 && lhs.revision == rhs.revision && lhs.world0 == rhs.world0) return true;
	else return false;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		/*if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}*/

		//Create window
		gWindow = SDL_CreateWindow("SDL Destruction", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_EVENT_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, "opengl");
			SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				////Initialize PNG loading
				//int imgFlags = IMG_INIT_PNG;
				//if (IMG_Init() < 0)
				//{
				//	printf("SDL_image could not initialize! SDL_image Error: %s\n", SDL_GetError());
				//	success = false;
				//}
			}
		}
		worldDef = b2DefaultWorldDef();
		worldDef.gravity = { 0.0f, 0.0f };
		worldId = b2CreateWorld(&worldDef);
		b2BodyDef groundBodyDef = b2DefaultBodyDef();
		groundBodyDef.position = { 320.0f*pixelsToMetres, 460.0f * pixelsToMetres };
		b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);
		b2Polygon groundBox = b2MakeBox(320.0f*pixelsToMetres, 5.0f*pixelsToMetres);
		b2ShapeDef groundShapeDef = b2DefaultShapeDef();
		b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
		statics.push_back(groundId);
	}
	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load Foo' texture
	if (!testTexture.loadPixelsFromFile("assets/foo.png"))
	{
		printf("Failed to load Foo' texture!\n");
		success = false;
	}
	else {

		if (!testTexture.loadFromPixels(gRenderer))
		{
			printf("Unable to load Foo' texture from surface!\n");
		}
	}

	//So that there is some sort of default collider to go along with a default texture.
	std::vector<int> points = { 0, (testTexture.getHeight() - 1) * testTexture.getWidth(), (testTexture.getHeight() * testTexture.getWidth()) - 1, testTexture.getWidth() - 1 };
	b2BodyId tempId = createTexturePolygon(points, testTexture.getWidth(), worldId, &testTexture);
	colliders.push_back(tempId);

	return success;
}

void close()
{
	//Free loaded images
	for (int i = 0; i < textures.size(); i++) {
		textures[i]->free();
	}
	b2DestroyWorld(worldId);
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	//IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[]) {

	if (!init()) {
		printf("Failed to initialize!\n");
	}
	else {
		if (!loadMedia()) {
			printf("Failed to load media!\n");
		}
		else {
			bool quit = false;
			bool leftMouseButtonDown = false;
			textures.push_back(&testTexture);//This works fine now.
			std::vector<std::vector<int>> testingPoints;
			std::vector<std::vector<int>> rdpPoints;
			bool getOutline = false;
			bool getSimplifiedOutline = false;
			bool getColliderOutlines = false;

			SDL_Event e;
			while (!quit) {
				int x, y;
				while (SDL_PollEvent(&e)) {
					switch (e.type) {
					case SDL_EVENT_QUIT:
						quit = true;
						break;
					//Reseting bool values
					case SDL_EVENT_MOUSE_BUTTON_UP:
						if (e.button.button == SDL_BUTTON_LEFT) {
							std::vector<Texture*> texturesToRemove;
							std::vector<Texture*> texturesToAdd;
							std::vector<b2BodyId> collidersToRemove;

							for(int i = 0; i < textures.size(); i++) {
								if (textures[i]->isAltered()) {
									for (Texture* texture : splitTextureAtEdge(textures[i], gRenderer)) {
										texturesToAdd.push_back(texture);
									}
									textures[i]->resetSplittingFlag();
									texturesToRemove.push_back(textures[i]);
									collidersToRemove.push_back(colliders[i]);
								}
							}

							for (Texture* t : texturesToAdd) {
								textures.push_back(t);

								std::vector<int> tempPoints = marchingSquares(t);

								std::vector<int> temprdpPoints;
								//Position at size()-2 is where 0 is stored. This will give us the 
								//straight line that we want. If we add origin at end as well it messes up partition so don't do that.
								temprdpPoints.push_back(tempPoints[tempPoints.size() - 2]);
								rdp(0, tempPoints.size() - 1, 3, t->getWidth(), tempPoints, temprdpPoints);
								colliders.push_back(createTexturePolygon(temprdpPoints, t->getWidth(), worldId, t));
							}
							texturesToAdd.clear();

							for (Texture* t : texturesToRemove) {
								textures.erase(find(textures.begin(), textures.end(), t));
								t->free();
							}
							texturesToRemove.clear();

							for (b2BodyId id : collidersToRemove) {
								colliders.erase(find(colliders.begin(), colliders.end(), id));
								b2DestroyBody(id);
							}
							collidersToRemove.clear();

							printf("Number of Textures: %i\n", textures.size());
							printf("Number of Colliders: %i\n", colliders.size());
							leftMouseButtonDown = false;
						}
						break;
					case SDL_EVENT_MOUSE_BUTTON_DOWN:
						if (e.button.button == SDL_BUTTON_LEFT) {
							leftMouseButtonDown = true;
							getOutline = false;
							getSimplifiedOutline = false;

							for (Texture* t : textures) {
								
								Vector2 rotated = rotateAboutPoint(newVector2(e.motion.x, e.motion.y), t->getCentre(), -t->getAngle(), false);
								if (rotated.x >= t->getOrigin().x && rotated.x < t->getOrigin().x + t->getWidth() &&
									rotated.y < t->getOrigin().y + t->getHeight() && rotated.y >= t->getOrigin().y
									) {
									erasePixels(t, gRenderer, scale, e.motion.x, e.motion.y);
								}
							}
						}
						break;
					case SDL_EVENT_MOUSE_MOTION:
						if (leftMouseButtonDown && e.motion.x >= 0 && e.motion.x < 640 && e.motion.y < 480 && e.motion.y >= 0) {
							for (Texture* t : textures) {
								Vector2 rotated = rotateAboutPoint(newVector2(e.motion.x, e.motion.y), t->getCentre(), -t->getAngle(), false);
								if (rotated.x >= t->getOrigin().x && rotated.x < t->getOrigin().x + t->getWidth() &&
									rotated.y < t->getOrigin().y + t->getHeight() && rotated.y >= t->getOrigin().y
									) {
									erasePixels(t, gRenderer, scale, e.motion.x, e.motion.y);
								}
							}
						}
						break;
					case SDL_EVENT_KEY_DOWN:
						if (e.key.key == SDLK_O) {
							getOutline = !getOutline;
							if (getOutline) {
								testingPoints.clear();
								for (int i = 0; i < textures.size(); i++) {
									std::vector<int> tempPoints = marchingSquares(textures[i]);
									testingPoints.push_back(tempPoints);
								}
							}
						}
						if (e.key.key == SDLK_S) {
							getSimplifiedOutline = !getSimplifiedOutline;
							if (getSimplifiedOutline) {
								rdpPoints.clear();
								for (int i = 0; i < textures.size(); i++) {
									std::vector<int> tempPoints;
									//Position at size()-2 is where 0 is stored, which is what we want. This will give us the 
									//straight line that we want.
									tempPoints.push_back(testingPoints[i][testingPoints[i].size() - 2]);
									rdp(0, testingPoints[i].size() - 1, 3, textures[i]->getWidth(), testingPoints[i], tempPoints);
									tempPoints.push_back(testingPoints[i][testingPoints[i].size() - 2]);
									rdpPoints.push_back(tempPoints);
								}
							}
						}
						if (e.key.key == SDLK_C) {
							getColliderOutlines = !getColliderOutlines;
						}
						if (e.key.key == SDLK_A && textures.size() == 1) {
							textures[0]->setAngle(textures[0]->getAngle() - 1);
							b2Rot angle = { cos(textures[0]->getAngle() * DEGREES_TO_RADIANS), sin(textures[0]->getAngle() * DEGREES_TO_RADIANS) };
							b2Body_SetTransform(colliders[0], b2Body_GetPosition(colliders[0]), angle);
						}
						if (e.key.key == SDLK_D && textures.size() == 1) {
							textures[0]->setAngle(textures[0]->getAngle() + 1);
							b2Rot angle = { cos(textures[0]->getAngle() * DEGREES_TO_RADIANS), sin(textures[0]->getAngle() * DEGREES_TO_RADIANS) };
							b2Body_SetTransform(colliders[0], b2Body_GetPosition(colliders[0]), angle);
						}
						if (e.key.key == SDLK_P) {
							for (int i = 0; i < textures.size(); i++) {
								printf("New Position = (%f, %f)\n", textures[i]->getOrigin().x, textures[i]->getOrigin().y);
								b2Vec2 position = b2Body_GetPosition(colliders[i]);
								printf("Expected New Position in meters = (%f, %f)\n", position.x, position.y);
								printf("Expected New Position in pixels = (%i, %i)\n", static_cast<int>(std::round(position.x* metresToPixels)), static_cast<int>(std::round(position.y* metresToPixels)));
							}
						}
						break;
					}
				}
				//This is where all the functionality in the main loop will go.
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				for (int i = 0; i < textures.size(); i++) {
					textures[i]->render(gRenderer, NULL, textures[i]->getAngle(), NULL, SDL_FLIP_NONE);
				}

				//Marching squares
				if (getOutline) {
					for (int i = 0; i < testingPoints.size(); i++) {
						for (int j = 0; j < testingPoints[i].size() - 1; j++) {
							SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF);
							SDL_RenderLine(gRenderer, textures[i]->getOrigin().x + (testingPoints[i][j] % textures[i]->getWidth()),
								textures[i]->getOrigin().y + (int)(floor(testingPoints[i][j] / textures[i]->getWidth())),
								textures[i]->getOrigin().x + (testingPoints[i][j + 1] % textures[i]->getWidth()),
								textures[i]->getOrigin().y + (int)(floor(testingPoints[i][j + 1] / textures[i]->getWidth())));
						}
					}
				}
				//rdp
				if (getSimplifiedOutline) {
					for (int i = 0; i < rdpPoints.size(); i++) {
						for (int j = 0; j < rdpPoints[i].size() - 1; j++) {
							SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
							SDL_RenderLine(gRenderer, textures[i]->getOrigin().x + (rdpPoints[i][j] % textures[i]->getWidth()),
								textures[i]->getOrigin().y + (int)(floor(rdpPoints[i][j] / textures[i]->getWidth())),
								textures[i]->getOrigin().x + (rdpPoints[i][j + 1] % textures[i]->getWidth()),
								textures[i]->getOrigin().y + (int)(floor(rdpPoints[i][j + 1] / textures[i]->getWidth())));
						}
					}
				}
				//Colliders
				if (getColliderOutlines) {
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);

					//This here is for drawing the "floor" collider at the bottom of the screen 
					b2ShapeId* shapes = new b2ShapeId[1];
					b2Body_GetShapes(statics[0], shapes, 1);
					//printf("PosX: %f, PosY: %f", b2Body_GetPosition(statics[0]).x*metresToPixels, b2Body_GetPosition(statics[0]).y*metresToPixels);
					b2Vec2 position = b2Body_GetPosition(statics[0]);
					b2Vec2* vertices = b2Shape_GetPolygon(shapes[0]).vertices;
					for (int i = 0; i < 4; i++) {
						SDL_RenderLine(gRenderer, ((vertices[i].x + position.x)* metresToPixels), ((vertices[i].y + position.y)* metresToPixels),
													((vertices[(i + 1) > 3 ? 0 : (i + 1)].x + position.x)* metresToPixels), ((vertices[(i + 1) > 3 ? 0 : (i + 1)].y + position.y)* metresToPixels));
					}

					//For drawing "normal" colliders. Assuming that all colliders are made up of triangles.
					//Triangles I think are the way forward, need to make it so all colliders are made up of triangle 
					//polygons. It just makes things easier.
					for (int i = 0; i < colliders.size(); i++) {
						int shapeCount = b2Body_GetShapeCount(colliders[i]);
						b2Vec2 colliderPosition = b2Body_GetPosition(colliders[i]);
						//printf("Collider Position: (%f, %f)", colliderPosition.x, colliderPosition.y);
						b2ShapeId* colliderShapes = new b2ShapeId[shapeCount];
						b2Body_GetShapes(colliders[i], colliderShapes, shapeCount);
						for (int j = 0; j < shapeCount; j++) {
							b2Vec2* colliderVertices = b2Shape_GetPolygon(colliderShapes[j]).vertices;
							for (int k = 0; k < 3; k++) {
								rotateTranslate(colliderVertices[k], b2Rot_GetAngle(b2Body_GetRotation(colliders[i])));
							}
							for (int k = 0; k < 3; k++) {
								SDL_RenderLine(gRenderer, ((colliderVertices[k].x + colliderPosition.x)* metresToPixels), ((colliderVertices[k].y + colliderPosition.y)* metresToPixels),
									((colliderVertices[(k + 1) > 2 ? 0 : (k + 1)].x + colliderPosition.x)* metresToPixels), ((colliderVertices[(k + 1) > 2 ? 0 : (k + 1)].y + colliderPosition.y)* metresToPixels));
							}
						}
					}
				}

				//Updating graphics
				SDL_RenderPresent(gRenderer);

				//Updating physics
				b2World_Step(worldId, 1.0f / 60.0f, 4);
				for (int i = 0; i < colliders.size(); i++) {
					b2Vec2 position = b2Body_GetPosition(colliders[i]);
					float angle = normalizeAngle(b2Rot_GetAngle(b2Body_GetRotation(colliders[i])));
					/*if (textures[i]->getCentre().x != static_cast<float>(position.x * metresToPixels) && textures[i]->getCentre().y != static_cast<float>(position.y * metresToPixels)) {
						printf("OLD CENTER: %f, %f\n", textures[i]->getCentre().x, textures[i]->getCentre().y);
						printf("NEW CENTER: %f, %f\n", textures[i]->getCentre().x, textures[i]->getCentre().y);
					}*/
					textures[i]->setCentre(static_cast<float>(position.x*metresToPixels), static_cast<float>(position.y* metresToPixels));
					textures[i]->setAngle(angle / DEGREES_TO_RADIANS);
				}
			}
		}
	}
	close();
	return 0;
}