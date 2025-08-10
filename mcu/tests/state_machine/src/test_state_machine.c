/**
 * @file    test_state_machine.c
 * @brief   Unit tests for state machine
 */

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "state_machine.h"

LOG_MODULE_REGISTER(test_sm, LOG_LEVEL_DBG);

/* ---------- Fixture ---------- */
static void *fsm_setup(void)
{
    sm_test_init();
    return NULL;
}

static void fsm_teardown(void *fixture)
{
    ARG_UNUSED(fixture);
}

/* ---------- Tests ---------- */
ZTEST(fsm_suite, test_idle_to_active_on_pi_start)
{
    /* Initially in IDLE (set by sm_test_init) */
    k_msleep(100);
    zassert_equal(sm_test_get_state(), STATE_IDLE, "Not in IDLE at start");

    /* Post start event and run one FSM step */
    sm_post(EVT_PI_START);
    sm_test_run_step();
    k_msleep(10);

    zassert_equal(sm_test_get_state(), STATE_ACTIVE, "Did not transition to ACTIVE");
}

ZTEST(fsm_suite, test_active_to_avoiding_on_obstacle)
{
    k_msleep(100);
    /* Initially in IDLE (set by sm_test_init) */
    zassert_equal(sm_test_get_state(), STATE_IDLE, "Not in IDLE at start");

    /* Go active first */
    sm_post(EVT_PI_START);
    sm_test_run_step();
    k_msleep(10);
    zassert_equal(sm_test_get_state(), STATE_ACTIVE, "Expected ACTIVE");

    /* Now simulate obstacle */
    sm_post(EVT_OBSTACLE);
    sm_test_run_step();
    k_msleep(10);

    zassert_equal(sm_test_get_state(), STATE_AVOIDING, "Expected AVOIDING");

    /* Reset state machine */
    sm_post(EVT_OBSTACLE_CLEAR);
    sm_test_run_step();
    k_msleep(10);
}

/* ---------- Suite Registration ---------- */
ZTEST_SUITE(fsm_suite, NULL, fsm_setup, NULL, fsm_teardown, NULL);