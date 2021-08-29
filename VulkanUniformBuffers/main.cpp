#include <memory>
#include "Engine.h"
#include "SdlWindow.h"


int main(int argc, char* args[]) 
{
	auto engine = std::make_shared<Engine>();
	SdlWindow sdlWindow(engine);
	sdlWindow.runMainLoop();
	return 0;
}