#include <iostream>
#include "SdlWindow.h"


SdlWindow::SdlWindow(std::shared_ptr<Engine> engine)
{
	SDL_Init(SDL_INIT_VIDEO);
	this->engine = engine;
	sdlWindow = buildSdlWindow();
	throwIfWindowCreationFailed(sdlWindow);
	this->engine->init(sdlWindow);
}

SdlWindow::~SdlWindow()
{
	engine->cleanUp();
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();
}

SDL_Window* SdlWindow::buildSdlWindow() const
{
	return SDL_CreateWindow(
		WINDOW_TITLE,
		WINDOW_X_POSITION,
		WINDOW_Y_POSITION,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		WINDOW_FLAGS
	);
}

void SdlWindow::throwIfWindowCreationFailed(SDL_Window* window)
{
	if (isWindowNotCreated(window))
	{
		std::cerr << "Cannot create SDL window!" << std::endl;
		exit(-1);
	}
}

bool SdlWindow::isWindowNotCreated(SDL_Window* window)
{
	return !window;
}

void SdlWindow::runMainLoop()
{
	SDL_Event sdlEvent;
	bool running = true;

	while (running)
	{
		dispatchSdlEventIfExist(&sdlEvent, &running);

		engine->update();
		engine->render();
	}
}

void SdlWindow::dispatchSdlEventIfExist(SDL_Event* sdlEvent, bool* outRunning)
{
	if (SDL_PollEvent(sdlEvent))
	{
		dispatchSdlEvent(sdlEvent, outRunning);
	}
}

void SdlWindow::dispatchSdlEvent(SDL_Event* sdlEvent, bool* outRunning)
{
	switch (sdlEvent->type)
	{
	case SDL_WINDOWEVENT:
		if (sdlEvent->window.event == SDL_WINDOWEVENT_CLOSE)
		{
			*outRunning = false;
		}
		break;
	default:
		engine->readInput(*sdlEvent);
	}
}