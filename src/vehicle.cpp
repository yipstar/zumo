//
// Created by yipstar on 2019-02-19.
//

#include <Arduino.h>
//#include <HardwareSerial.h>
#include "qpn.h"
#include "app.h"

typedef struct Vehicle {
    QActive super;
} Vehicle;

// forward declare state handlers
static QState Vehicle_initial(Vehicle * const me);

static QState Vehicle_off(Vehicle * const me);
static QState Vehicle_on(Vehicle * const me);

static QState Vehicle_disarmed(Vehicle * const me);

Vehicle AO_Vehicle;

void Vehicle_ctor(void) {
    // do nothing
    Vehicle *me = &AO_Vehicle;
    QActive_ctor(&me->super, Q_STATE_CAST(&Vehicle_initial));
}

QState Vehicle_initial(Vehicle * const me) {
  /*QActive_armX((QActive *)me, 0U,
               BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);*/

  return Q_TRAN(&Vehicle_disarmed);
}

static QState Vehicle_disarmed(Vehicle * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      Serial.println("Vehicle_disarmed");
      /*digitalWrite(LED_L, LOW);*/
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      /*Serial.println("Vehicle_off timeout");*/
      return Q_TRAN(&Vehicle_on);
    }
  }

}

static QState Vehicle_off(Vehicle * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      /*Serial.println("Vehicle_off");*/
      digitalWrite(LED_L, LOW);
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      /*Serial.println("Vehicle_off timeout");*/
      return Q_TRAN(&Vehicle_on);
    }
  }

  return Q_SUPER(&QHsm_top);
}

static QState Vehicle_on(Vehicle * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
 /*     Serial.println("Vehicle_on");*/
      digitalWrite(LED_L, HIGH);
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      /*Serial.println("Vehicle_on timeout");*/
      return Q_TRAN(&Vehicle_off);
    }
  }

  return Q_SUPER(&QHsm_top);
}