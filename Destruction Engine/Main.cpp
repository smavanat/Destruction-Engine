#include<SDL3_image/SDL_image.h>
#include <chrono>
#include "Outline.h"
#include "BasicSystems.h"
#include "TileSystem.h"
#include "Debug.h"
#include "GridData.h"
#include "GridManager.h"
#include "point.h"
#include "polygon.h"
#include "martinez.h"

//TODO: Figure out how do deal with small shapes. Colliders are not generated for them, but they are still there.
//		Maybe just erase them? Or put a default small collider around them.

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int FRAME_RATE = 1000/60.0f;

////Some global variables
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
b2WorldDef worldDef;
b2WorldId worldId;

//ECS Managers
Coordinator gCoordinator;
DebugManager gDebugManager;
GridSystemManager gGridManager;

//ECS systems
std::shared_ptr<RenderSystem> renderSystem;
std::shared_ptr<TransformSystem> transformSystem;
std::shared_ptr<DestructionSystem> destructionSystem;
std::shared_ptr<TileRenderSystem> tileSystem;
TileSet t;
TerrainSet tSet;
//std::shared_ptr<TileSystem> tileSystem;

//Test entities;
Entity testTexture;
//Entity testPath;
Entity testAgent;

int scale = 20;

bool init();
bool loadMedia();

//Necessary to check which colliders to remove from vector since c++ does not generate default comparators for structs.
bool operator ==(const b2BodyId& lhs, const b2BodyId& rhs) {
	if (lhs.index1 == rhs.index1 && lhs.generation == rhs.generation && lhs.world0 == rhs.world0) return true;
	else return false;
}

bool init()
{
	gCoordinator = Coordinator();
	gDebugManager = DebugManager();

	{
		Signature sig;
		sig.addComponent<Transform>();
		sig.addComponent<Sprite>();
		renderSystem = gCoordinator.addSystem<RenderSystem>(sig);
	}

	{
		Signature sig;
		sig.addComponent<Transform>();
		sig.addComponent<Collider>();
		transformSystem = gCoordinator.addSystem<TransformSystem>(sig);
	}

	{
		Signature sig;
		sig.addComponent<Transform>();
		sig.addComponent<Sprite>();
		sig.addComponent<Collider>();
		sig.addComponent<Terrain>();
		destructionSystem = gCoordinator.addSystem<DestructionSystem>(sig);
	}

	{
		Signature sig;
		sig.addComponent<Transform>();
		sig.addComponent<TileSprite>();
		tileSystem = gCoordinator.addSystem<TileRenderSystem>(sig);
	}
	gGridManager = GridSystemManager(TILE_WIDTH, GRID_WIDTH, GRID_HEIGHT);

	//Initialise all the systems.
	gCoordinator.init();
	gDebugManager.init(); //In case Debug systems/manager need some other form of initialisation

	testTexture = gCoordinator.createEntity();
	//testPath = gCoordinator.createEntity();
	testAgent = gCoordinator.createEntity();
	//Creating the tileset
	Sprite* srcSprite = (Sprite*)malloc(sizeof(Sprite));
	*srcSprite = Sprite(nullptr, nullptr, false);
	t = (TileSet){srcSprite, std::vector<TileClip*>(), std::vector<TileClip*>()};

	//Creating the array that holds all of the terrain colliders
	tSet = (TerrainSet){(uint32_t*)calloc(25, sizeof(uint32_t)), 0, 25};
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
			SDL_SetRenderVSync(gRenderer, SDL_RENDERER_VSYNC_ADAPTIVE); 
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
			}
		}
		gCoordinator.addComponent(testTexture, Transform((Vector2){1420.0f, 440.0f}, 0.0));
		gCoordinator.addComponent(testTexture, Terrain(false));
		//gCoordinator.addComponent(testPath, Pathfinding((Vector2){10, 10}, (Vector2){500, 500}, 2));
		gCoordinator.addComponent(testAgent, Transform((Vector2){40, 40}, 0.0f));
		gCoordinator.addComponent(testAgent, Terrain(false));
		gCoordinator.addComponent(testAgent, Pathfinding((Vector2){10, 10}, (Vector2){500, 500}, 2));

		worldDef = b2DefaultWorldDef();
		worldDef.gravity = { 0.0f, 0.0f };
		worldId = b2CreateWorld(&worldDef);
	}
	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;
	gCoordinator.addComponent(testTexture, Sprite(NULL, NULL, false));
	Sprite &s = gCoordinator.getComponent<Sprite>(testTexture);
	//Load Foo' texture
	if (!loadPixelsFromFile(s, "assets/foo.png"))
	{
		printf("Failed to load Foo' texture!\n");
		success = false;
	}
	else {
		if (!loadFromPixels(s, gRenderer))
		{
			printf("Unable to load Foo' texture from surface!\n");
		}
	}

	Sprite as = Sprite(NULL, NULL, false);
	if (!loadPixelsFromFile(as, "assets/TestSprite.png"))
	{
		printf("Failed to load TestSprite' texture!\n");
		success = false;
	}
	else {
		if (!loadFromPixels(as, gRenderer))
		{
			printf("Unable to load TestSprite' texture from surface!\n");
		}
	}
	gCoordinator.addComponent(testAgent, as);

	if (!initialiseDemoTileMap(t, gRenderer, "assets/MarchingSquares.png", "assets/Pathfinding.map", &tSet)) {
		printf("Unable to load tileset\n");
		success = false;
	}

	if (!gGridManager.loadGridFromFile("assets/Pathfinding.map")) {
		printf("Unable to load grid\n");
		success = false;
	}

	//gGridManager.setGridTileColliders(&tSet);

	//So that there is some sort of default collider to go along with a default texture.
	Transform t = gCoordinator.getComponent<Transform>(testTexture);
	b2BodyId tempId = createBoxCollider(t.position, s.surfacePixels->w, s.surfacePixels->h, t.rotation, worldId, b2_staticBody);
	gCoordinator.addComponent(testTexture, Collider(tempId, BOX));
	t = gCoordinator.getComponent<Transform>(testAgent);
	tempId = createCircleCollider(t.position, 20, worldId, b2_dynamicBody);
	gCoordinator.addComponent(testAgent, Collider(tempId, CIRCLE));

	return success;
}

void close()
{
	for (int i = 0; i < MAX_ENTITIES; i++) {
		Entity e;
		e.id = i;
		gCoordinator.destroyEntity(e);
	}
	b2DestroyWorld(worldId);
	freeTileSet(t);
	free(tSet.eArr);
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
			float dt = 0.0f;

			SDL_Event e;
			while (!quit) {
				//This is where all the functionality in the main loop will go.
				int x, y;
				while (SDL_PollEvent(&e)) {
					switch (e.type) {
						case SDL_EVENT_QUIT:
							quit = true;
							break;
						case SDL_EVENT_MOUSE_BUTTON_UP:
							if (e.button.button == SDL_BUTTON_LEFT) {
								gCoordinator.getInput()->leftMouseButtonDown = false;
								//Pathfinding &p = gCoordinator.getComponent<Pathfinding>(testPath);
								//p.startPos = (Vector2){10, 10};
								//p.endPos = (Vector2){500, 500};
								gCoordinator.getEventBus()->publish(new ErasureEvent());
								break;
							}
						case SDL_EVENT_MOUSE_BUTTON_DOWN:
							if (e.button.button == SDL_BUTTON_LEFT) {
								gCoordinator.getInput()->leftMouseButtonDown = true;
								gCoordinator.getInput()->mouseX = e.motion.x;
								gCoordinator.getInput()->mouseY = e.motion.y;
								break;
							}

						case SDL_EVENT_MOUSE_MOTION:
							if (gCoordinator.getInput()->leftMouseButtonDown) {
								gCoordinator.getInput()->mouseX = e.motion.x;
								gCoordinator.getInput()->mouseY = e.motion.y;
								break;
							}

						case SDL_EVENT_KEY_DOWN:
							if (e.key.key == SDLK_C) {
								gCoordinator.getEventBus()->publish(new ColliderDebugEvent());
							}
							if (e.key.key == SDLK_G) {
								//gGridManager.printWorldGrid();
								gCoordinator.getEventBus()->publish(new GridDebugEvent());
							}
							if (e.key.key == SDLK_P) {
								gCoordinator.getEventBus()->publish(new PathFindingDebugEvent());
							}
					}
				}

				auto startTime = std::chrono::high_resolution_clock::now();

				destructionSystem->update(dt);
				tileSystem->update(dt);
				gGridManager.update(dt);
				b2World_Step(worldId, 1.0f/60.0f, 4);
				transformSystem->update(dt);

				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
				renderSystem->update(dt);
				gDebugManager.update(dt);
				SDL_RenderPresent(gRenderer); //Need to put this outside the render system update since need to call it after both render and debug have drawn

				auto stopTime = std::chrono::high_resolution_clock::now();
				dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
				if(dt < FRAME_RATE) {
					//This reduces cpu usage from like 10-12% down to 0.6%
					SDL_Delay(FRAME_RATE - dt);
					dt = FRAME_RATE;
				}
			}
		}
	}
	close();
	return 0;
}