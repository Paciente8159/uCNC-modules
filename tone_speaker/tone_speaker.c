/*
	Name: tone_speaker.c
	Description: A simple module that makes your machine play sounds

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 14/01/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#if defined(ENABLE_BOOT_JINGLE) || defined(ENABLE_TONE_SPEAKER)

#if (UCNC_MODULE_VERSION < 10800 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#define NOTE_DUTY_CYCLE 24

// The default boot jingle
#if defined(ENABLE_BOOT_JINGLE) && !defined(BOOT_JINGLE_TUNE)
// Define the boot jingle as an array of pairs of unsigned shorts.
// First short specifies the note duration and the second one it's pitch.
#define BOOT_JINGLE_TUNE { 250, 1567, 125, 1174, 125, 1318, 125, 1567 };
#endif // defined(ENABLE_BOOT_JINGLE) && !defined(BOOT_JINGLE_TUNE)

#if !defined(SPEAKER_PWM)
#error "Please specify a PWM pin for the tone speaker"
#endif // !defined(SPEAKER_PWM)

#if defined(ENABLE_BOOT_JINGLE)
const uint16_t jingle[] = BOOT_JINGLE_TUNE;
#define JINGLE_LENGTH (sizeof(jingle) / sizeof(uint16_t))
#else // !defined(ENABLE_BOOT_JINGLE)
const uint16_t jingle[0];
#define JINGLE_LENGTH 0
#endif // defined(ENABLE_BOOT_JINGLE)

static uint16_t currentNote;
static uint32_t targetMillis;
static bool playing;

void play_note(uint16_t frequency, uint16_t duration) {
#if ASSERT_PIN(SPEAKER_PWM)
	if(frequency > 0) {
		// Configure new frequency...
		io_config_pwm(SPEAKER_PWM, frequency);
		// ...and generate a square wave
		io_set_pwm(SPEAKER_PWM, NOTE_DUTY_CYCLE);
	} else {
		// Frequency 0 = no signal
		io_set_pwm(SPEAKER_PWM, 0);
	}
#endif // ASSERT_PIN(SPEAKER_PWM)

	targetMillis = mcu_millis() + duration;
	playing = true;
}

#if defined(ENABLE_MAIN_LOOP_MODULES)

bool speaker_note_update(void* args) {
	if(mcu_millis() >= targetMillis) {
#if defined(ENABLE_BOOT_JINGLE)
		if(currentNote >= JINGLE_LENGTH) {
			// Ran out of notes
			if(playing) {
#if ASSERT_PIN(SPEAKER_PWM)
				io_set_pwm(SPEAKER_PWM, 0);
#endif // ASSERT_PIN(SPEAKER_PWM)
				playing = false;
			}
			return EVENT_CONTINUE;
		}

		// Play next note
		uint16_t frequency = jingle[currentNote + 1];
		play_note(frequency, jingle[currentNote]);
		currentNote += 2;
#else // !defined(ENABLE_BOOT_JINGLE)
		// Reached target millis, stop playing note
		if(playing) {
			io_set_pwm(SPEAKER_PWM, 0);
			playing = false;
		}
#endif // defined(ENABLE_BOOT_JINGLE)
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, speaker_note_update);

#else // !defined(ENABLE_MAIN_LOOP_MODULES)
#error "Please enable main loop modules or the tone speaker module will not function correctly"
#endif // defined(ENABLE_MAIN_LOOP_MODULES)

#if defined(ENABLE_TONE_SPEAKER)
#if !defined(ENABLE_PARSER_MODULES)
#error "Tone speaker requires the parser modules to be enabled"
#endif // !defined(ENABLE_PARSER_MODULES)

#define M300 EXTENDED_MCODE(300)

bool tone_mcode_parser(void* args) {
	gcode_parse_args_t* gargs = (gcode_parse_args_t*)args;
	if(gargs->word == 'M' && gargs->value == 300) {
		if(gargs->cmd->group_extended != 0) {
			*(gargs->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}

		gargs->cmd->group_extended = M300;
		*(gargs->error) = STATUS_OK;
		// Consume the event
		return EVENT_HANDLED;
	}
	return EVENT_CONTINUE;
}

bool tone_mcode_executor(void* args) {
	gcode_exec_args_t * gargs = (gcode_exec_args_t*)args;
	if(gargs->cmd->group_extended == M300) {
		uint16_t time = gargs->words->p;
		uint16_t frequency = gargs->words->s;

		if(playing) {
			// Wait for the current note to stop playing
			cnc_delay_ms(targetMillis - mcu_millis());
		}

		play_note(frequency, time);

		*(gargs->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(gcode_parse, tone_mcode_parser);
CREATE_EVENT_LISTENER(gcode_exec, tone_mcode_executor);
#endif // defined(ENABLE_TONE_SPEAKER)

DECL_MODULE(tone_speaker)
{
	targetMillis = mcu_millis() + 1000;
	currentNote = 0;
	playing = false;

#if ASSERT_PIN(SPEAKER_PWM)
	io_config_pwm(SPEAKER_PWM, 1);
	io_set_pwm(SPEAKER_PWM, 0);
#endif // ASSERT_PIN(SPEAKER_PWM)

#if defined(ENABLE_MAIN_LOOP_MODULES)
	ADD_EVENT_LISTENER(cnc_dotasks, speaker_note_update);
#else // !defined(ENABLE_MAIN_LOOP_MODULES)
#error "Main loop extensions are not enabled. Speaker will not work."
#endif // defined(ENABLE_MAIN_LOOP_MODULES)

#if defined(ENABLE_PARSER_MODULES) && defined(ENABLE_TONE_SPEAKER)
	ADD_EVENT_LISTENER(gcode_parse, tone_mcode_parser);
	ADD_EVENT_LISTENER(gcode_exec, tone_mcode_executor);
#endif // defined(ENABLE_PARSER_MODULES) && defined(ENABLE_TONE_SPEAKER)
}

#endif // defined(ENABLE_BOOT_JINGLE) || defined(ENABLE_TONE_SPEAKER)

