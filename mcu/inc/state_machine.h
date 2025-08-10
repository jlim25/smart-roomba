/**
 ********************************************************************************
 * @file    state_machine.h
 * @author  Jacky Lim
 * @date    July 2025
 * @brief   State machine header for Smart Roomba MCU
 ********************************************************************************
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

/************************************
 * INCLUDES
 ************************************/
#include <stdint.h>
/************************************
 * MACROS AND DEFINES
 ************************************/
/* Define event bits */
#define EVT_PI_START        BIT(0)  // Pi says: start moving
#define EVT_PI_STOP         BIT(1)  // Pi says: stop moving
#define EVT_OBSTACLE        BIT(2)  // Sensor: obstacle detected
#define EVT_OBSTACLE_CLEAR  BIT(3)  // Sensor: obstacle cleared
#define EVT_LOW_BATTERY     BIT(4)  // Battery: critically low
#define EVT_DOCK_FOUND      BIT(5)  // IR sensor: dock detected

/************************************
 * TYPEDEFS
 ************************************/
/* Define state enum */
typedef enum {
    STATE_IDLE,
    STATE_ACTIVE,   // Following Pi's command
    STATE_FAULT,
    STATE_AVOIDING,
    STATE_MAX
} robot_state_t;

/************************************
 * EXPORTED VARIABLES
 ************************************/

/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/
/* Public API */
void sm_post(uint32_t event_bits);        /* thread or ISR safe */
void sm_post_obstacle(void);              /* convenience helpers (optional) */
void sm_post_obstacle_clear(void);
void sm_post_low_battery(void);

/* Test API */
int sm_test_get_state(void);
void sm_test_init(void);        /* initializes FSM but doesn't start forever loop */
void sm_test_run_step(void);    /* calls smf_run_state() once */
#endif // STATE_MACHINE_H