/*
 * joystick_task.h
 *
 *  Created on: Jun 14, 2025
 *      Author: vunam
 */

#ifndef APPLICATION_USER_JOYSTICK_TASK_H_
#define APPLICATION_USER_JOYSTICK_TASK_H_

#include "cmsis_os.h"


typedef enum {
    JOY1_LEFT,
    JOY1_RIGHT,
    JOY1_UP,
	JOY1_DOWN,
    JOY2_LEFT,
    JOY2_RIGHT,
    JOY2_UP,
	JOY2_DOWN,
	JOY1_BUTTON,

	JOY2_BUTTON
} JoystickCommand_t;

#ifdef __cplusplus
extern "C" {
#endif

void JoystickTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_USER_JOYSTICK_TASK_H_ */
