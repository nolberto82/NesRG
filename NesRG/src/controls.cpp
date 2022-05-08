#include "controls.h"
#include "sdlgfx.h"

u8 controls_keys(u8 id)
{
	switch (id)
	{
		case 0:
			return sdl.ctrl_keys[SDL_SCANCODE_Z];
		case 1:
			return sdl.ctrl_keys[SDL_SCANCODE_X];
		case 2:
			return sdl.ctrl_keys[SDL_SCANCODE_SPACE];
		case 3:
			return sdl.ctrl_keys[SDL_SCANCODE_RETURN];
		case 4:
			return sdl.ctrl_keys[SDL_SCANCODE_UP];
		case 5:
			return sdl.ctrl_keys[SDL_SCANCODE_DOWN];
		case 6:
			return sdl.ctrl_keys[SDL_SCANCODE_LEFT];
		case 7:
			return sdl.ctrl_keys[SDL_SCANCODE_RIGHT];

	}

	return 0;
}

u8 controls_pad(u8 id)
{
	switch (id)
	{
		case 0:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_B);
		case 1:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_A);
		case 2:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_BACK);
		case 3:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_START);
		case 4:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
		case 5:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		case 6:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		case 7:
			return SDL_GameControllerGetButton(sdl.controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);	

	}

	return 0;
}

void controls_write(u8 val)
{
	strobe = val & 1;

	if (strobe)
	{
		buttonid = 0;
	}
}

u8 controls_read()
{
	u8 val = 0x40;

	if (strobe == 0 && buttonid >= 0)
	{
		if (buttonid >= 8)
		{
			buttonid = 0;
		}

		if (controls_pad(buttonid))
		{
			val = 0x41;
		}

		buttonid++;
	}
	return val;
}
