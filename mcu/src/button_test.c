/**
 * @file    button_test.c
 * @brief   Button testing for state machine
 * @details Uses the blue user button to cycle through FSM events
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "state_machine.h"

LOG_MODULE_REGISTER(button_test, LOG_LEVEL_DBG);

/* Button definitions for STM32F4-DISC1 */
#define USER_BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(USER_BUTTON_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

/* Test event sequence */
static uint32_t test_events[] = {
    BIT(0),  // EVT_PI_START
    BIT(2),  // EVT_OBSTACLE  
    BIT(3),  // EVT_OBSTACLE_CLEAR
    BIT(1),  // EVT_PI_STOP
    BIT(4),  // EVT_LOW_BATTERY
    BIT(0),  // EVT_PI_START (again)
};

static uint8_t event_index = 0;

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    LOG_INF("🔘 Button pressed! Sending event %d", event_index);
    
    /* Post the current test event */
    sm_post(test_events[event_index]);

    /* Move to next event (cycle through) */
    event_index = (event_index + 1) % ARRAY_SIZE(test_events);
    
    LOG_INF("Next button press will send event %d", event_index);
}

int button_init(void)
{
    int ret;

    if (!gpio_is_ready_dt(&button)) {
        LOG_ERR("Button device not ready");
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Failed to configure button pin: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Failed to configure button interrupt: %d", ret);
        return ret;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    LOG_INF("🔘 Button initialized successfully");
    LOG_INF("Press button to cycle through events:");
    LOG_INF("  Event 0: PI_START");
    LOG_INF("  Event 1: OBSTACLE");
    LOG_INF("  Event 2: OBSTACLE_CLEAR");
    LOG_INF("  Event 3: PI_STOP");
    LOG_INF("  Event 4: LOW_BATTERY");
    LOG_INF("  Event 5: PI_START (repeat)");

    return 0;
}

/* Initialize button on startup */
// SYS_INIT(button_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY); // this function is executed on start up