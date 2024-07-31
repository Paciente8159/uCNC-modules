#define AXIS_COUNT 3
#define STEPPER_COUNT 3

#define __romstr__(str) (str)

#define MSG_STATUS_ALARM __romstr__("Alarm")
#define MSG_STATUS_DOOR __romstr__("Door")
#define MSG_STATUS_HOLD __romstr__("Hold")
#define MSG_STATUS_HOME __romstr__("Home")
#define MSG_STATUS_JOG __romstr__("Jog")
#define MSG_STATUS_RUN __romstr__("Run")
#define MSG_STATUS_IDLE __romstr__("Idle")
#define MSG_STATUS_CHECK __romstr__("Check")

#define STR_ALARM_0 "Unknown"
#define STR_ALARM_1 "Hard limits hit"
#define STR_ALARM_2 "Soft limits hit"
#define STR_ALARM_3 "Aborted by user"
#define STR_ALARM_4 "Probe hit"
#define STR_ALARM_5 "Probe no hit"
#define STR_ALARM_6 "Home reset"
#define STR_ALARM_7 "Door opened"
#define STR_ALARM_8 "Limits in contact"
#define STR_ALARM_9 "Failed home find"
#define STR_ALARM_10 "Failed autolevel"
#define STR_ALARM_11 "Limits are active"
#define STR_ALARM_12 "Tool sync fail"
#define STR_ALARM_13 "Limits tripped"

#define STR_MOVEMENT "Movement"

#define EXEC_IDLE 0																						 // All flags cleared
#define EXEC_RUN 1																						 // Motions are being executed
#define EXEC_HOLD 2																						 // Feed hold is active
#define EXEC_JOG 4																						 // Jogging in execution
#define EXEC_HOMING 8																					 // Homing in execution
#define EXEC_DOOR 16																					 // Safety door open
#define EXEC_UNHOMED 32																				 // Machine is not homed or lost position due to abrupt stop
#define EXEC_LIMITS 64																				 // Limits hit
#define EXEC_KILL 128																					 // Emergency stop
#define EXEC_HOMING_HIT (EXEC_HOMING | EXEC_LIMITS)						 // Limit switch is active during a homing motion
#define EXEC_INTERLOCKING_FAIL (EXEC_LIMITS | EXEC_KILL)			 // Interlocking check failed
#define EXEC_ALARM (EXEC_UNHOMED | EXEC_INTERLOCKING_FAIL)		 // System alarms
#define EXEC_RESET_LOCKED (EXEC_ALARM | EXEC_DOOR | EXEC_HOLD) // System reset locked
#define EXEC_GCODE_LOCKED (EXEC_ALARM | EXEC_DOOR | EXEC_JOG)	 // Gcode is locked by an alarm or any special motion state
#define EXEC_ALLACTIVE 255																		 // All states

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

void rom_strcpy(char* dest, const char* src) {
	while(*src)
		*dest++ = *src++;
	*dest = 0;
}

#define CNC_STATE EXEC_IDLE
uint8_t cnc_get_exec_state(uint8_t state) {
	return (CNC_STATE) & state;
}

bool cnc_has_alarm() {
	return (CNC_STATE) & EXEC_ALARM;
}

uint8_t cnc_get_alarm() {
	return 0;
}

void parser_get_modes(uint8_t *modes, uint16_t *feed, uint16_t *spindle) {
	modes[6] = 54;
	modes[2] = 90;
	modes[4] = 21;
	modes[11] = 1;
	*feed = 0;
	*spindle = 0;
}

void itp_get_rt_position(int32_t* steppos) {
	for(int i = 0; i < STEPPER_COUNT; ++i)
		steppos[i] = 0;
}

void kinematics_apply_forward(int32_t *steppos, float *mpos) {
	for(int i = 0; i < STEPPER_COUNT; ++i)
		mpos[i] = steppos[i];
}

void kinematics_apply_reverse_transform(float *mpos) {}
void parser_machine_to_work(float *wpos) { }

#ifndef MAX_MODAL_GROUPS
#define MAX_MODAL_GROUPS 14
#endif

#include "../../../style/win9x/utility.h"

#include "../../../style/win9x/fonts/symbols_8x8.h"
#include "../../../style/win9x/fonts/freesans9pt7b.h"
#include "../../../style/win9x/fonts/freemonobold12pt7b.h"
#include "../../../style/win9x/fonts/pixel.h"
#include "../../../style/win9x/fonts/pixelmono.h"

