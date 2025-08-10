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
#endif // STATE_MACHINE_H