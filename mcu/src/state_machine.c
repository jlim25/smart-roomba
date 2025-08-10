/**
 * @file    state_machine.c
 * @brief   Smart Roomba MCU state machine implementation
 * @author  Jacky Lim
 * @date    July 2025
 * @details This file implements the state machine for the Smart Roomba MCU
 * 
 */
#include "state_machine.h"

#include <zephyr/kernel.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(state_machine, LOG_LEVEL_DBG); // Change to LOG_LEVEL_DBG for later

/* Define state enum */
typedef enum {
    STATE_IDLE,
    STATE_ACTIVE,   // Following Pi's command
    STATE_FAULT,
    STATE_AVOIDING,
    STATE_MAX
} robot_state_t;

/* FSM context */
struct robot {
    struct smf_ctx ctx;
    
    /* User defined variables */
    struct k_event evt; // Kernel event object
    uint32_t events;
    bool motors_on;
    bool vacuum_on;
};

/* Define FSM instance */
static struct robot robo_fsm;
static const struct smf_state state_table[STATE_MAX];

/* ---------- STATE: IDLE ---------- */
static void idle_entry(void *o)
{
    LOG_DBG("Entering IDLE state\n");

    // struct robot *r = (struct robot *)o;
    // LOG_INF("STM32: IDLE - motors stopped");
    
    // stop_motors();
    // r->motors_on = false;
    // r->vacuum_on = false;
    // report_to_pi("IDLE");
}

static void idle_run(void *o)
{
    struct robot *r = (struct robot *)o;
    if (r->events & EVT_PI_START) {
        LOG_INF("In idle_run function\n");
        smf_set_state(SMF_CTX(r), &state_table[STATE_ACTIVE]);
    }
}

static void idle_exit(void *o)
{
    LOG_DBG("Exiting IDLE state\n");
}

/* ---------- STATE: Active ---------- */
static void active_entry(void *o)
{
    LOG_DBG("Entering active state\n");

    // struct robot *r = (struct robot *)o;
    // LOG_INF("STM32: MOVING - following Pi commands");
    
    // start_motors();
    // r->motors_on = true;
    // r->vacuum_on = true;
    // report_to_pi("MOVING");
}

static void active_run(void *o)
{
    struct robot *r = (struct robot *)o;

    if (r->events & EVT_OBSTACLE) {
        LOG_WRN("SAFETY: Obstacle detected - overriding Pi");
        smf_set_state(SMF_CTX(r), &state_table[STATE_AVOIDING]);
        // TODO: how will the pi handle this?
    }

    if (r->events & EVT_PI_STOP) {
        LOG_INF("In active_run function\n");
        smf_set_state(SMF_CTX(r), &state_table[STATE_IDLE]);
    }

    if (r->events & EVT_LOW_BATTERY) {
        LOG_WRN("BATTERY: Low battery - returning to dock");
        smf_set_state(SMF_CTX(r), &state_table[STATE_IDLE]);
    }
}

static void active_exit(void *o)
{
    LOG_DBG("Exiting ACTIVE state\n");
}

/* ---------- STATE: AVOIDING ---------- */
static void avoiding_entry(void *o)
{
    LOG_DBG("Entering avoiding state\n");

    // struct robot *r = (struct robot *)o;
    // LOG_WRN("STM32: AVOIDING - safety override active");
    
    // stop_motors();  // Immediate stop
    // r->motors_on = false;
    // report_to_pi("AVOIDING_OBSTACLE");
}

static void avoiding_run(void *o)
{
    struct robot *r = (struct robot *)o;

    if (r->events & EVT_OBSTACLE_CLEAR) {
        LOG_INF("Obstacle cleared - returning control to Pi");
        smf_set_state(SMF_CTX(r), &state_table[STATE_IDLE]);
    }
    /* Pi can override if it has a plan */
    if (r->events & EVT_PI_START) {
        LOG_INF("Pi override - resuming movement");
        smf_set_state(SMF_CTX(r), &state_table[STATE_ACTIVE]);
    }
}

static void avoiding_exit(void *o) {
    LOG_DBG("Leaving AVOIDING\n");
}

/* State table - much simpler! */
static const struct smf_state state_table[STATE_MAX] = {
    [STATE_IDLE] = SMF_CREATE_STATE(idle_entry, idle_run, idle_exit, NULL, NULL),
    [STATE_ACTIVE] = SMF_CREATE_STATE(active_entry, active_run, active_exit, NULL, NULL),
    [STATE_AVOIDING] = SMF_CREATE_STATE(avoiding_entry, avoiding_run, avoiding_exit, NULL, NULL)
};

/* ---- Public posting API (other files call these) ---- */
void sm_post(uint32_t event_bits)
{
    /* k_event_post is ISR-safe; no extra locking needed */
    k_event_post(&robo_fsm.evt, event_bits);
}

/* Typed helpers for readability */
void sm_post_obstacle(void)       { sm_post(EVT_OBSTACLE); }
void sm_post_obstacle_clear(void) { sm_post(EVT_OBSTACLE_CLEAR); }
void sm_post_low_battery(void)    { sm_post(EVT_LOW_BATTERY); }


void fsm_thread(void)
{
    int ret = 0;
    LOG_INF("Starting FSM thread...");

    // Initialize the FSM
    memset(&robo_fsm, 0, sizeof(robo_fsm));
    k_event_init(&robo_fsm.evt);

    smf_set_initial(SMF_CTX(&robo_fsm), &state_table[STATE_IDLE]);
    LOG_INF("STM32 state machine initialized and ready. In Idle state.");

    /* Mask of all events this thread should react to */
    const uint32_t ALL_EVENTS =
        EVT_PI_START | EVT_PI_STOP |
        EVT_OBSTACLE | EVT_OBSTACLE_CLEAR |
        EVT_LOW_BATTERY | EVT_DOCK_FOUND;

    while (1) {
        robo_fsm.events = k_event_wait(&robo_fsm.evt, ALL_EVENTS, /*reset=*/true, K_FOREVER);
        LOG_DBG("FSM thread woke up with events: 0x%08X", robo_fsm.events);
        ret = smf_run_state(SMF_CTX(&robo_fsm));

        if (ret) {
            LOG_ERR("State machine error: %d", ret);
            // Handle error, maybe reset or log
            break;
        }
    }
}

/* ---------- Main Entry ---------- */
K_THREAD_DEFINE(fsm_tid, 1024, fsm_thread, NULL, NULL, NULL, 5, 0, 0);  // This defines the thread and starts it immediately