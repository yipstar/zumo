//
// Created by yipstar on 2019-02-19.
//

#ifndef ZUMOSM_APP_H
#define ZUMOSM_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qpn.h"

// various constants for the application...
enum {
    BSP_TICKS_PER_SEC = 100, // number of syvoid Vehicle_ctor(void);stem clock ticks in one second
    LED_L = 13, // the pin number of the on-board LED (L)
    MODE_PIN = 11,
    THROTTLE_PIN = 4,
    STEERING_PIN = 5,
    PULSE_WIDTH_DEADBAND = 25, // pulse width difference from 1500 us (microseconds) to ignore (to compensate for control centering offset)
    PULSE_WIDTH_RANGE = 350, // pulse width difference from 1500 us to be treated as full scale input (for example, a value of 350 means any pulse width <= 1150 us or >= 1850 us is considered full scale)
    MAX_SPEED = 400 // max motor speed
};

extern struct Vehicle AO_Vehicle;
extern struct RC AO_RC;

void Vehicle_ctor(void);
void RC_ctor(void);

// log facility experiments
//enum eLogSubSystem {
//  RC, VEHICLE
//};
//
//enum eLogLevel {
//  DEBUG, WARN, SYS
//};
//
//void Log(enum eLogSubSystem sys, enum eLogLevel level, char *msg);

#ifdef __cplusplus
}
#endif

#endif //ZUMOSM_APP_H
