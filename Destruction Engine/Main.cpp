#include<SDL3_image/SDL_image.h>
#include <chrono>
#include "Outline.hpp"
#include "BasicSystems.hpp"
#include "TileSystem.h"
#include "Debug.h"

//TODO: Figure out how do deal with small shapes. Colliders are not generated for them, but they are still there.
//		Maybe just erase them? Or put a default small collider around them.

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

////Some global variables
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
b2WorldDef worldDef;
b2WorldId worldId;

//ECS Managers
Coordinator gCoordinator;
DebugManager gDebugManager;

//ECS systems
std::shared_ptr<RenderSystem> renderSystem;
std::shared_ptr<DestructionSystem> destructionSystem;
std::shared_ptr<TileSystem> tileSystem;
std::shared_ptr<GridSystem> gridSystem;
std::shared_ptr<PathFindingSystem> pathfindingSystem;

//Test entities;
Entity testTexture;
Entity testPath;

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
		sig.addComponent<Sprite>();
		sig.addComponent<Collider>();
		destructionSystem = gCoordinator.addSystem<DestructionSystem>(sig);
	}

	{
		Signature sig;
		sig.addComponent<Transform>();
		sig.addComponent<TileType>();
		tileSystem = gCoordinator.addSystem<TileSystem>(sig);
	}

	{
		Signature sig;
		sig.addComponent<Transform>();
		sig.addComponent<Walkable>();
		gridSystem = gCoordinator.addSystem<GridSystem>(sig);
	}

	{
		Signature sig;
		sig.addComponent<Pathfinding>();
		pathfindingSystem = gCoordinator.addSystem<PathFindingSystem>(sig);
	}
	//Initialise all the systems.
	gCoordinator.init();
	gDebugManager.init(); //In case Debug systems/manager need some other form of initialisation

	testTexture = gCoordinator.createEntity();
	testPath = gCoordinator.createEntity();
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
		gCoordinator.addComponent(testTexture, Transform(newVector2(1420.0f, 440.0f), 0.0));
		gCoordinator.addComponent(testPath, Pathfinding(newVector2(10, 10), newVector2(500, 500)));

		gridSystem->updatePathfinding(); //This line needs to exist otherwise the pathfinding will not have the initial grid

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
	gCoordinator.addComponent(testTexture, Sprite(NULL, NULL, newVector2(1420.0f, 440.0f), 0, 0, 0.0, false));
	Sprite &s = gCoordinator.getComponent<Sprite>(testTexture);
	//Load Foo' texture
	if (!s_loadPixelsFromFile(s, "assets/foo.png"))
	{
		printf("Failed to load Foo' texture!\n");
		success = false;
	}
	else {
		if (!s_loadFromPixels(s, gRenderer))
		{
			printf("Unable to load Foo' texture from surface!\n");
		}
	}

	if (!tileSystem->loadTileSet()) {
		printf("Unable to load tileset\n");
		success = false;
	}

	//So that there is some sort of default collider to go along with a default texture.
	std::vector<int> points = { 0, (s.height - 1) * s.width, (s.height * s.width) - 1, s.width - 1 };
	b2BodyId tempId = s_createTexturePolygon(points, s.width, worldId, s);
	gCoordinator.addComponent(testTexture, Collider(tempId));

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
								gCoordinator.getEventBus()->publish(new GridDebugEvent());
							}
							if (e.key.key == SDLK_P) {
								gCoordinator.getEventBus()->publish(new PathFindingDebugEvent());
							}
					}
				}

				auto startTime = std::chrono::high_resolution_clock::now();

				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
				destructionSystem->update(dt);
				gridSystem->update(dt);
				pathfindingSystem->update(dt);
				tileSystem->update(dt);
				renderSystem->update(dt);
				gDebugManager.update(dt);
				SDL_RenderPresent(gRenderer); //Need to put this outside the render system update since need to call it after both render and debug have drawn

				auto stopTime = std::chrono::high_resolution_clock::now();

				dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
			}
		}
	}
	close();
	return 0;
}