/*
	Name: config.h
	Description: Graphics library for µCNC

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 20/07/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#ifndef GRAPHICS_LIBRARY_CONFIG_H
#define GRAPHICS_LIBRARY_CONFIG_H

#define GFX_RENDER_BUFFER_SIZE (32 * 1024)

#ifdef GFX_TESTING
#include ".tests/test_driver.h"

#define GFX_AFTER_ELEMENT() 
#define GFX_CTX_TIME() 0
#else
#include "../../../cnc.h"
#include "../driver/ILI9486.h"

#define GFX_AFTER_ELEMENT_HOOK() if(mcu_millis() - ctx->sc_time >= 3) \
{ \
	cnc_dotasks(); \
	ctx->sc_time = mcu_millis(); \
}

#define GFX_CTX_TIME() mcu_millis()
#endif

#endif
