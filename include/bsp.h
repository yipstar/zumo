//
// Created by yipstar on 2019-02-19.
//

#ifndef ZUMOSM_BSP_H
#define ZUMOSM_BSP_H

#include "app.h"

// interrupts...x
ISR(TIMER2_COMPA_vect) {
  // process time events for tick rate 0
  QF_tickXISR(0);
}

// QF callbacks...
void QF_onStartup(void) {
    // set Timer2 in CTC mode, 1/1024 prescaler, start the timer ticking...
    TCCR2A = (1U << WGM21) | (0U << WGM20);
    TCCR2B = (1U << CS22 ) | (1U << CS21) | (1U << CS20); // 1/2^10
    ASSR  &= ~(1U << AS2);
    TIMSK2 = (1U << OCIE2A); // enable TIMER2 compare Interrupt
    TCNT2  = 0U;

    // set the output-compare register based on the desired tick frequency
    OCR2A  = (F_CPU / BSP_TICKS_PER_SEC / 1024U) - 1U;
}

void QV_onIdle(void) {   // called with interrupts DISABLED
    // Put the CPU and peripherals to the low-power mode. You might
    // need to customize the clock management for your application,
    // see the datasheet for your particular AVR MCU.
    SMCR = (0 << SM0) | (1 << SE); // idle mode, adjust to your project
    QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
}

void Q_onAssert(char const Q_ROM * const file, int line) {
    // implement the error-handling policy for your application!!!
    QF_INT_DISABLE(); // disable all interrupts
    QF_RESET();  // reset the CPU
}

/* Check for the minimum required QP version */
#if ((QP_VERSION < 601) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8)))
#error qpn version 6.0.1 or higher required
#endif


#endif //ZUMOSM_BSP_H
