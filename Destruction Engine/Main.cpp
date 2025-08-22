#include<SDL3_image/SDL_image.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>
#include "BasicComponents.h"
#include "BasicSystems.h"
#include "Entity.h"
#include "Event.h"
#include "Maths.h"
#include "SDL3/SDL_rect.h"
#include "Signature.h"
#include "TileSystem.h"
#include "Debug.h"
#include "GridData.h"
#include "GridManager.h"
#include "box2d/box2d.h"
#include "box2d/id.h"

//TODO: Figure out how do deal with small shapes. Colliders are not generated for them, but they are still there.
//		Maybe just erase them? Or put a default small collider around them.

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const int FRAME_RATE = 1000/60.0f;

////Some global variables
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
b2WorldDef worldDef;
b2WorldId worldId;

//ECS Managers
Coordinator gCoordinator;
DebugManager gDebugManager;
std::shared_ptr<GridData> grid;
// GridSystemManager gGridManager;

//ECS systems
std::shared_ptr<RenderSystem> renderSystem;
std::shared_ptr<TransformSystem> transformSystem;
std::shared_ptr<DestructionSystem> destructionSystem;
std::shared_ptr<TileRenderSystem> tileSystem;
std::shared_ptr<UISystem> uiSystem;
std::shared_ptr<PathFindingSystem> pSystem;
TileSet t;
TerrainSet tSet;

//Test entities;
Entity testAgent;
Entity b1;
Entity b2;
Entity b3;
Sprite as;

int scale = 30;

bool init();
bool loadMedia();

bool initD1();
bool initD2();
bool initD3();

bool loadD1();
bool loadD2();
bool loadD3();

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

    {
        Signature sig;
        sig.addComponent<Button>();
        uiSystem = gCoordinator.addSystem<UISystem>(sig);
    }
    {
        Signature sig;
        sig.addComponent<Pathfinding>();
        pSystem = gCoordinator.addSystem<PathFindingSystem>(sig);
    }
    grid = std::make_shared<GridData>();
    CreateGridData(grid, TILE_WIDTH, GRID_WIDTH, GRID_HEIGHT);
    pSystem->setGrid(grid);

	//Initialise all the systems.
	gCoordinator.init();
	gDebugManager.init(); //In case Debug systems/manager need some other form of initialisation

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
        b1 = gCoordinator.createEntity();
        b2 = gCoordinator.createEntity();
        b3 = gCoordinator.createEntity();

        SDL_FRect* t1 = (SDL_FRect*)malloc(sizeof(SDL_FRect));
        *t1 = (SDL_FRect){1400, 200, 100, 50};
        SDL_FRect* t2 = (SDL_FRect*)malloc(sizeof(SDL_FRect));
        *t2 = (SDL_FRect){1400, 400, 100, 50};
        SDL_FRect* t3 = (SDL_FRect*)malloc(sizeof(SDL_FRect));
        *t3 = (SDL_FRect){1400, 600, 100, 50};

        gCoordinator.addComponent(b1, Button(t1, FIRST));
        gCoordinator.addComponent(b2, Button(t2, SECOND));
        gCoordinator.addComponent(b3, Button(t3, THIRD));

        gCoordinator.addComponent(b1, Transform((Vector2){1450, 225}, 0.0f));
        gCoordinator.addComponent(b2, Transform((Vector2){1450, 425}, 0.0f));
        gCoordinator.addComponent(b3, Transform((Vector2){1450, 625}, 0.0f));

        worldDef = b2DefaultWorldDef();
        worldDef.gravity = { 0.0f, 0.0f };
        worldId = b2CreateWorld(&worldDef);
	}
	return success;
}

bool loadSprite(Sprite& s, std::string path) {
	if (!loadPixelsFromFile(s, path))
	{
		printf("Failed to load %s texture!\n", path.data());
        return false;
	}
	else {
		if (!loadFromPixels(s, gRenderer))
		{
			printf("Unable to load %s' texture from surface!\n", path.data());
            return false;
		}
	}
    return true;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;
	as = Sprite(NULL, NULL, false);
    Sprite sb1 = Sprite(NULL, NULL, false);
    Sprite sb2 = Sprite(NULL, NULL, false);
    Sprite sb3 = Sprite(NULL, NULL, false);
    // success = loadSprite(as, "assets/TestSprite.png");
    success = loadSprite(sb1, "assets/b1.png");
    success = loadSprite(sb2, "assets/b2.png");
    success = loadSprite(sb3, "assets/b3.png");
    success = initialiseDemoTileMap(t, gRenderer, "assets/MarchingSquares.png", "assets/Pathfinding2.map", &tSet);

    gCoordinator.addComponent(b1, sb1);
    gCoordinator.addComponent(b2, sb2);
    gCoordinator.addComponent(b3, sb3);

	return success;
}


bool initAgent(bool basic) {
    testAgent = gCoordinator.createEntity();

   gCoordinator.addComponent(testAgent, Transform((Vector2){40, 40}, 0.0f));
   gCoordinator.addComponent(testAgent, Pathfinding((Vector2){10, 10}, (Vector2){1240, 920}, 2));

    if(!basic) {
        Transform t = gCoordinator.getComponent<Transform>(testAgent);
        b2BodyId tempId = createCircleCollider(t.position, 20, worldId, b2_dynamicBody);
        gCoordinator.addComponent(testAgent, Collider(tempId, CIRCLE));
    }
    return true;
}

bool loadMap(std::string path, bool addAgent) {
    if(addAgent) {
        if(!loadSprite(as, "assets/TestSprite.png")) {
            printf("Unable to load agent sprite\n");
            return false;
        }
        gCoordinator.addComponent(testAgent, as);
    }

	if (!loadTileMapFromFile(t, gRenderer, path, &tSet)) {
		printf("Unable to load tileset\n");
		return false;
	}

	if (!LoadGridFromFile(grid, path)) {
		printf("Unable to load grid\n");
		return false;
	}
    return true;
}

void closeDemo() {
    std::vector<Entity> entitiesToDestroy = std::vector<Entity>();
    for(Entity e : destructionSystem->registeredEntities) {
        Sprite s = gCoordinator.getComponent<Sprite>(e);
        if(s.surfacePixels != NULL)
            free(s.surfacePixels);
        Collider c = gCoordinator.getComponent<Collider>(e);
        b2DestroyBody(c.colliderId);
        entitiesToDestroy.push_back(e);
        // gCoordinator.destroyEntity(e);
    }
    for(int i = 0; i < entitiesToDestroy.size(); i++) {
        gCoordinator.destroyEntity(entitiesToDestroy[i]);
    }

    grid->tiles.clear();
    grid->tiles = std::vector<TileData>(static_cast<size_t>(grid->gridHeight* grid->gridWidth));
    as = Sprite(NULL, NULL, false);
    gCoordinator.destroyEntity(testAgent);
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
            initAgent(true);
            loadMap("assets/Pathfinding2.map", false);

			bool quit = false;
			float dt = 0.0f;
            DEMO_STATE cState = gCoordinator.state;

			SDL_Event e;
			while (!quit) {
				//This is where all the functionality in the main loop will go.
				// int x, y;
				while (SDL_PollEvent(&e)) {
					switch (e.type) {
						case SDL_EVENT_QUIT:
							quit = true;
							break;
						case SDL_EVENT_MOUSE_BUTTON_UP:
							if (e.button.button == SDL_BUTTON_LEFT) {
                                gCoordinator.getInput()->leftMouseButtonDown = false;
                                if(gCoordinator.state != THIRD) {
                                    gCoordinator.getEventBus()->publish(new ErasureEvent());
                                }
								break;
							}
						case SDL_EVENT_MOUSE_BUTTON_DOWN:
							if (e.button.button == SDL_BUTTON_LEFT) {
                                bool old = gCoordinator.getInput()->leftMouseButtonDown;
								gCoordinator.getInput()->leftMouseButtonDown = true;
								gCoordinator.getInput()->mouseX = e.motion.x;
								gCoordinator.getInput()->mouseY = e.motion.y;
                                if(old == false) {
                                    gCoordinator.getEventBus()->publish(new ClickedEvent(Vector2{e.motion.x, e.motion.y}));
                                }
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
                if(gCoordinator.state != cState) {
                    closeDemo();
                    cState = gCoordinator.state;
                    switch (cState) {
                        case FIRST:
                            initAgent(true);
                            loadMap("assets/Pathfinding2.map", false);
                            break;
                        case SECOND:
                            initAgent(false);
                            loadMap("assets/Pathfinding2.map", true);
                            break;
                        case THIRD:
                            initAgent(false);
                            loadMap("assets/Pathfinding.map", true);
                            break;
                    }
                }

				auto startTime = std::chrono::high_resolution_clock::now();

				destructionSystem->update(dt);
                pSystem->update(dt);
				b2World_Step(worldId, 1.0f/60.0f, 4);
				transformSystem->update(dt);

				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
				tileSystem->update(dt);
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
