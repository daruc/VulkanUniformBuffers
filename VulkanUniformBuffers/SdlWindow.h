#pragma once

#include <memory>
#include "SDL.h"
#include "Engine.h"


class SdlWindow
{
private:
	const int WINDOW_X_POSITION = SDL_WINDOWPOS_UNDEFINED;
	const int WINDOW_Y_POSITION = SDL_WINDOWPOS_UNDEFINED;
	const char* WINDOW_TITLE = "Vulkan Uniform Buffers";
	const Uint32 WINDOW_FLAGS = SDL_WINDOW_VULKAN;
	const int WINDOW_HEIGHT = 600;
	const int WINDOW_WIDTH = 800;

	std::shared_ptr<Engine> engine;
	SDL_Window* sdlWindow;

	SDL_Window* buildSdlWindow() const;
	void throwIfWindowCreationFailed(SDL_Window* window);
	bool isWindowNotCreated(SDL_Window* window);
	void dispatchSdlEventIfExist(SDL_Event* sdlEvent, bool* outRunning);
	void dispatchSdlEvent(SDL_Event* sdlEvent, bool* outRunning);

public:
	SdlWindow(std::shared_ptr<Engine> engine);
	~SdlWindow();
	void runMainLoop();
};