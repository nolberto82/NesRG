#include "controls.h"
#include "renderer.h"

u8 controls_keys(u8 id)
{
	switch (id)
	{
		case 0:
			return gfx.ctrl_keys[SDL_SCANCODE_Z];
		case 1:
			return gfx.ctrl_keys[SDL_SCANCODE_X];
		case 2:
			return gfx.ctrl_keys[SDL_SCANCODE_SPACE];
		case 3:
			return gfx.ctrl_keys[SDL_SCANCODE_RETURN];
		case 4:
			return gfx.ctrl_keys[SDL_SCANCODE_UP];
		case 5:
			return gfx.ctrl_keys[SDL_SCANCODE_DOWN];
		case 6:
			return gfx.ctrl_keys[SDL_SCANCODE_LEFT];
		case 7:
			return gfx.ctrl_keys[SDL_SCANCODE_RIGHT];

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

		if (controls_keys(buttonid))
		{
			val = 0x41;
		}

		buttonid++;
	}
	return val;
}
