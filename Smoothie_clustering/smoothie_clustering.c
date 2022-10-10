/*
	Name: smoothie_clustering.c
	Description: Enables Smoothieware S Clustering for LightBurn µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 27-09-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

#ifndef S_CLUSTER_SIZE
#define S_CLUSTER_SIZE 8
#endif

static uint16_t s_cluster[(S_CLUSTER_SIZE - 1)];
static uint8_t s_cluster_count;

#ifdef ENABLE_SYSTEM_INFO
uint8_t smoothie_clustering_info(void *args, bool *handled)
{
	protocol_send_string(__romstr__("CLUSTER:" STRGIFY(S_CLUSTER_SIZE)));
	return 0;
}

CREATE_EVENT_LISTENER(protocol_send_cnc_info, smoothie_clustering_info);
#endif

#ifdef ENABLE_PARSER_MODULES

uint8_t smoothie_clustering_parse_token(void *args, bool *handled)
{
	unsigned char c = *((unsigned char *)args);
	uint8_t i = 0;
	while (c == ':')
	{
		*handled = true;
		float val;
		if (!parser_get_float(&val))
		{
			return 0;
		}

		s_cluster[i++] = (uint16_t)(val * g_settings.spindle_max_rpm);
		c = serial_getc();
		if (c == EOL)
		{
			*((unsigned char *)args) = EOL;
			s_cluster_count = i;
			return 1;
		}
	}
	return 0;
}

CREATE_EVENT_LISTENER(parse_token, smoothie_clustering_parse_token);

uint8_t smoothie_clustering_gcode_exec_modifier(void *args, bool *handled)
{
	gcode_exec_args_t *gcode = (gcode_exec_args_t *)args;

	gcode->words->s *= g_settings.spindle_max_rpm;
	return 0;
}

CREATE_EVENT_LISTENER(gcode_exec_modifier, smoothie_clustering_gcode_exec_modifier);

#endif

#ifdef ENABLE_MOTION_CONTROL_MODULES

uint8_t smoothie_clustering_mc_line_segment(void *args, bool *handled)
{
	uint8_t clusters = s_cluster_count;
	s_cluster_count = 0;

	if (clusters)
	{
		motion_data_t *block_data = (motion_data_t *)args;
		motion_data_t new_block;
		memcpy(&new_block, block_data, sizeof(motion_data_t));

		for (uint8_t i = STEPPER_COUNT; i != 0;)
		{
			i--;
			new_block.steps[i] /= (clusters + 1);
		}

		new_block.total_steps /= (clusters + 1);
		block_data->dwell = 0;

		for (uint8_t j = 0; j < clusters; j++)
		{
			planner_add_line(&new_block);
			// dwell should only execute on the first request

			new_block.dwell = 0;

			for (uint8_t i = STEPPER_COUNT; i != 0;)
			{
				i--;
				block_data->steps[i] -= new_block.steps[i];
			}

			block_data->total_steps -= new_block.total_steps;
			block_data->spindle = new_block.spindle = s_cluster[j];

			while (planner_buffer_is_full())
			{
				if (!cnc_dotasks())
				{
					return STATUS_CRITICAL_FAIL;
				}
			}
		}
	}

	return 0;
}

CREATE_EVENT_LISTENER(mc_line_segment, smoothie_clustering_mc_line_segment);

#endif

DECL_MODULE(smoothie_clustering)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(parse_token, smoothie_clustering_parse_token);
	ADD_EVENT_LISTENER(gcode_exec_modifier, smoothie_clustering_gcode_exec_modifier);
#else
#warning "Parser extensions are not enabled. Smoothieware S Cluster module will not work."
#endif
#ifdef ENABLE_MOTION_CONTROL_MODULES
	ADD_EVENT_LISTENER(mc_line_segment, smoothie_clustering_mc_line_segment);
#else
#warning "Motion control extensions are not enabled. Smoothieware S Cluster module will not work."
#endif
#ifdef ENABLE_SYSTEM_INFO
	ADD_EVENT_LISTENER(protocol_send_cnc_info, smoothie_clustering_info);
#endif
}
