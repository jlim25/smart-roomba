
/**
 * @file    scheduler.c
 * @brief   Smart Roomba MCU scheduler implementation
 * @author  Jacky Lim
 * @date    August 2025
 * @details This file implements the scheduler for the Smart Roomba MCU. The defined control loops
 *          use a hardware timer to trigger a 1 kHz loop, which derives 100 Hz and 10 Hz loops.
 *          Relative timing is used to ensure that the 100 Hz and 10 Hz loops are phase-locked to the 1 kHz loop.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sched, LOG_LEVEL_INF);

/* ----- Semaphores for periodic loops ----- */
K_SEM_DEFINE(sem_1khz, 0, K_SEM_MAX_LIMIT);
K_SEM_DEFINE(sem_100hz, 0, K_SEM_MAX_LIMIT);
K_SEM_DEFINE(sem_10hz, 0, K_SEM_MAX_LIMIT);

/* ----- Aperiodic IPC ----- */
#define RX_MSGQ_ITEM_SZ   32
#define RX_MSGQ_DEPTH     16
K_MSGQ_DEFINE(rx_msgq, RX_MSGQ_ITEM_SZ, RX_MSGQ_DEPTH, 4);

/* ----- Timer alarm (1 kHz) ----- */
static const struct device *timer = DEVICE_DT_GET(DT_ALIAS(counter0));
static struct counter_alarm_cfg alarm_cfg;
static volatile uint32_t div_100 = 0, div_10 = 0;

static uint32_t period_ticks = 0; // Will be set in init

static void alarm_cb(const struct device *dev, uint8_t chan_id, uint32_t ticks, void *user_data)
{
    ARG_UNUSED(dev); ARG_UNUSED(chan_id); ARG_UNUSED(ticks); ARG_UNUSED(user_data);

    /* Release 1 kHz loop */
    k_sem_give(&sem_1khz);

    /* Derive 100 Hz and 10 Hz from same tick for phase-lock */
    /* We derive 100 Hz and 10 Hz this way to prevent drift */
    if (++div_100 >= 10) { 
        div_100 = 0; 
        k_sem_give(&sem_100hz); 
    }
    if (++div_10  >= 100){ 
        div_10  = 0; 
        k_sem_give(&sem_10hz);  
    }

    /* Re-arm next alarm (periodic) */
    alarm_cfg.flags = COUNTER_ALARM_CFG_EXPIRE_WHEN_LATE; /* absolute time */
    alarm_cfg.ticks = period_ticks;
    int ret = counter_set_channel_alarm(dev, 0, &alarm_cfg);
    if (ret != 0) {
            LOG_ERR("Failed to set alarm: %d", ret);
    }
}

/* ----- Periodic threads ----- */
static void loop_1khz(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    while (1) {
        k_sem_take(&sem_1khz, K_FOREVER);

        /* BEGIN 1 kHz critical section (keep < ~60–70% of 1ms budget) */
        // read encoders (non-blocking / cached)
        // run inner motor PID
        // write PWM setpoints
        /* END */

        /* Avoid any logging here; if needed, count and log at 10 Hz */
    }
}

static void loop_100hz(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    static uint32_t counter_100hz = 0;
    while (1) {
        k_sem_take(&sem_100hz, K_FOREVER);

        if ((counter_100hz++ % 100) == 0) {
            LOG_DBG("100Hz loop executed");
        }
        // IMU read (cached or via server thread)
        // state estimation / EKF step
        // trajectory tracking outer loop
        // LOG_DBG("100Hz loop executed");
    }
}

static void loop_10hz(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

    static uint32_t cnt = 0;
    while (1) {
        k_sem_take(&sem_10hz, K_FOREVER);

        // housekeeping, BMS polling, health, publishing slow telemetry
        if ((cnt++ % 10) == 0) {
            LOG_DBG("10Hz: health ok");
        }
    }
}

/* ----- Aperiodic worker ----- */
static void worker_rx(void *a, void *b, void *c)
{
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    uint8_t buf[RX_MSGQ_ITEM_SZ];

    // This thread processes incoming messages from the RX queue
    // Only runs when there are messages in the queue
    while (1) {
        if (k_msgq_get(&rx_msgq, buf, K_FOREVER) == 0) {
            LOG_DBG("Processing message");
            // parse/process one item (bounded work)
            // if heavy, schedule to system workqueue
        }
    }
}

/* ----- ISR example: UART RX bottom-half enqueue ----- */
void uart_rx_isr_handler(const uint8_t *data, size_t len)
{
    /* Copy minimal data; drop on overflow (non-blocking) */
    size_t cpy = MIN(len, RX_MSGQ_ITEM_SZ);
    uint8_t buf[RX_MSGQ_ITEM_SZ];
    memcpy(buf, data, cpy);

    // Put the message into the queue
    if (k_msgq_put(&rx_msgq, buf, K_NO_WAIT) != 0) {
        LOG_WRN("Message queue full, dropping data");
        /* optional: increment drop counter */
    }
}

/* ----- Init & thread creation ----- */
K_THREAD_STACK_DEFINE(stack_1k, 2048);
K_THREAD_STACK_DEFINE(stack_100, 2048);
K_THREAD_STACK_DEFINE(stack_10, 2048);
K_THREAD_STACK_DEFINE(stack_rx, 2048);
static struct k_thread th_1k, th_100, th_10, th_rx;

static int scheduler_init(void)
{
    LOG_INF("Initializing scheduler...");

    /* Counter init at 1 kHz alarms */
    if (!device_is_ready(timer)) {
        LOG_ERR("Counter device not ready: %s", timer->name);
        return -ENODEV;
    }

    /* Calculate period_ticks for 1kHz (1ms period) */
    period_ticks = counter_us_to_ticks(timer, 1000); /* 1000 microseconds */
    if (period_ticks == 0) {
        LOG_ERR("Invalid period calculation");
        return -EINVAL;
    }

    LOG_DBG("Timer frequency: %u Hz, period_ticks for 1kHz: %u", 
            counter_get_frequency(timer), period_ticks);

    /* Configure alarm for 1kHz (1ms period) */
    alarm_cfg.flags = COUNTER_ALARM_CFG_EXPIRE_WHEN_LATE; /* relative time */
    alarm_cfg.callback = alarm_cb;
    alarm_cfg.user_data = NULL;
    alarm_cfg.ticks = period_ticks;

    /* Start the counter */
    int ret = counter_start(timer);
    if (ret != 0) {
        LOG_ERR("Failed to start counter: %d", ret);
        return ret;
    }
    
    // Set alarm
    ret = counter_set_channel_alarm(timer, 0, &alarm_cfg);
    if (ret != 0) {
        LOG_ERR("Failed to set initial alarm: %d", ret);
        return ret;
    }

    /* Create threads with rate-monotonic priority */
    k_thread_create(&th_1k, stack_1k, K_THREAD_STACK_SIZEOF(stack_1k),
                    loop_1khz, NULL, NULL, NULL, 0 /* highest prio */, 0, K_NO_WAIT);
    k_thread_name_set(&th_1k, "loop_1k");

    k_thread_create(&th_100, stack_100, K_THREAD_STACK_SIZEOF(stack_100),
                    loop_100hz, NULL, NULL, NULL, 1, 0, K_NO_WAIT);
    k_thread_name_set(&th_100, "loop_100");

    k_thread_create(&th_10, stack_10, K_THREAD_STACK_SIZEOF(stack_10),
                    loop_10hz, NULL, NULL, NULL, 2, 0, K_NO_WAIT);
    k_thread_name_set(&th_10, "loop_10");

    k_thread_create(&th_rx, stack_rx, K_THREAD_STACK_SIZEOF(stack_rx),
                    worker_rx, NULL, NULL, NULL, 3, 0, K_NO_WAIT);
    k_thread_name_set(&th_rx, "worker_rx");

    LOG_INF("Scheduler initialized successfully");
    return 0;
}

SYS_INIT(scheduler_init, APPLICATION, 50);
