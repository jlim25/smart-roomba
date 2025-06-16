/**
 * @file    main.c
 * @brief   Smart Roomba MCU main file
 * @author  Jacky Lim
 * @date    June 2025
 * @details This file initializes the LEDs on the STM32F4 Discovery board
 * 
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(smart_roomba, LOG_LEVEL_DBG);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifiers for all 4 LEDs on STM32F4 Discovery */
#define LED0_NODE DT_ALIAS(led0)  /* Green LED (LD4) */
#define LED1_NODE DT_ALIAS(led1)  /* Orange LED (LD3) */
#define LED2_NODE DT_ALIAS(led2)  /* Red LED (LD5) */
#define LED3_NODE DT_ALIAS(led3)  /* Blue LED (LD6) */

/*
 * GPIO specifications for all 4 LEDs on STM32F4 Discovery board
 */
static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(LED0_NODE, gpios),  /* Green LED */
	GPIO_DT_SPEC_GET(LED1_NODE, gpios),  /* Orange LED */
	GPIO_DT_SPEC_GET(LED2_NODE, gpios),  /* Red LED */
	GPIO_DT_SPEC_GET(LED3_NODE, gpios),  /* Blue LED */
};

#define NUM_LEDS ARRAY_SIZE(leds)

int main(void)
{
    // Always available - Zephyr's logging system
    LOG_INF("Smart Roomba MCU starting...");
    
#ifdef DEBUG
    // Only in CMake Debug builds - your custom debug code for application level code
    LOG_DBG("Debug build - enabling verbose diagnostics");
    // Custom debug initialization
    // debug_print_system_info();
    // enable_performance_monitoring();
#endif

#ifdef CONFIG_DEBUG  
    // Only when Zephyr debug is enabled - system debug.
	// Used for kernel debugging, assertions, etc.
    LOG_DBG("Zephyr debug features enabled");
    // This will show thread info, kernel assertions, etc.
#endif

	int ret;
	static int led_pattern = 0;

	LOG_INF("Smart Roomba MCU starting up...");
	LOG_INF("Initializing all LEDs on STM32F4 Discovery board");

	/* Initialize all LEDs */
	for (int i = 0; i < NUM_LEDS; i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			LOG_ERR("LED %d device not ready", i);
			return 0;
		}

		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			LOG_ERR("Failed to configure LED %d pin", i);
			return 0;
		}
		
		LOG_INF("LED %d initialized successfully", i);
	}

	LOG_INF("Smart Roomba MCU initialization complete - all LEDs ready");

	while (1) {
		/* Turn off all LEDs first */
		for (int i = 0; i < NUM_LEDS; i++) {
			gpio_pin_set_dt(&leds[i], 0);
		}

		/* Create a rotating pattern - turn on LEDs one by one */
		gpio_pin_set_dt(&leds[led_pattern], 1);
		
		const char* led_names[] = {"Green", "Orange", "Red", "Blue"};
		LOG_INF("LED pattern: %s LED ON", led_names[led_pattern]);
		
		led_pattern = (led_pattern + 1) % NUM_LEDS;
		
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
