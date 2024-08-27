/*
	Name: config.h
	Description: Configures the Win9x style

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 14/08/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
*/

#ifndef WIN9X_CONFIG_H
#define WIN9X_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

// Include the "style selector" config
#include "../config.h"

// Enable a menu for moving into specific coordinates
// This requires a working numeric keypad.
#define MOVEMENT_MENU 0

// Enable a custom jogging menu
#define CUSTOM_JOG_MENU 0

#ifdef __cplusplus
}
#endif

#endif

