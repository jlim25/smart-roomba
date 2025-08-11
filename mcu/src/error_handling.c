#include <zephyr/kernel.h>
#include <zephyr/logging/log_ctrl.h>   // log_panic(), log_process()
// #include <zephyr/power/reboot.h>       // sys_reboot()
#include <zephyr/irq.h>

// To invoke a fatal error, use: k_panic(), or k_oops() [kills current thread], or __ASSERT();
// See: https://docs.zephyrproject.org/latest/kernel/services/other/fatal.html

__attribute__((noreturn))
// This function is defined weak in Zephyr and we overrides the default handler here
void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
    ARG_UNUSED(esf);

    /* 1) Enter panic mode: backends block; pending logs are flushed */
    log_panic();
    while (log_process()) {
        /* drain remaining messages */
    }

    /* 2) Put the system in a safe mode */
    // motors_disable();   // your functions
    // arm_brake();

    /* 3) Choose your policy: halt forever (matches your requirement) */
    k_fatal_halt(reason);  // never returns

    /* Alternative: reboot instead */
    // sys_reboot(SYS_REBOOT_COLD);
    // CODE_UNREACHABLE;
}
