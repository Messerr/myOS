#include "timer.h"
#include "isr.h"
#include "io.h"

static uint32_t tick_count = 0;

/* Forward declaration — the scheduler calls this */
extern void schedule(void);

static void timer_handler(struct interrupt_frame* frame) {
    (void)frame;
    tick_count++;
    schedule();
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

void timer_init(uint32_t frequency) {
    register_interrupt_handler(32, timer_handler);  /* IRQ0 = interrupt 32 */

    /* PIT runs at 1193180 Hz internally. We divide to get our desired rate. */
    uint32_t divisor = 1193180 / frequency;

    outb(0x43, 0x36);                    /* Channel 0, lobyte/hibyte, rate generator */
    outb(0x40, (uint8_t)(divisor & 0xFF));       /* Low byte */
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF)); /* High byte */
}
