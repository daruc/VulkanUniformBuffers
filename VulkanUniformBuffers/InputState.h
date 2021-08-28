#pragma once

#include "SDL.h"

struct InputState
{
	bool forward;
	bool backward;
	bool left;
	bool right;
	Sint32 mouseXRel;
	Sint32 mouseYRel;
	bool mouseRight;
};