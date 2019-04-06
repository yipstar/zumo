#include <stdbool.h>

#include <Arduino.h>
#include "qpn.h"
#include "app.h"

typedef struct Distance {
    QActive super;
    uint16_t current_range;
} Distance;

// forward declare state handlers
static QState Distance_initial(Distance * const me);
static QState Distance_top(Distance * const me);
static QState Distance_detected(Distance * const me);

Distance AO_Distance;

void Distance_ctor(void) {
  // do nothing
  Distance *me = &AO_Distance;
  me->current_range = 0;
  QActive_ctor(&me->super, Q_STATE_CAST(&Distance_initial));
}

QState Distance_initial(Distance * const me) {
  QActive_armX((QActive *)me, 0U,
               BSP_TICKS_PER_SEC/10U, BSP_TICKS_PER_SEC/10U);

  return Q_TRAN(&Distance_top);
}

uint16_t get_gp2d12 (uint16_t value) {
  if (value < 10) value = 10;
  return ((67870.0 / (value - 3.0)) - 40.0);
}

uint16_t getRange() {
  uint16_t value = analogRead(DIST_PIN);
  uint16_t range = get_gp2d12(value);
  return range;
}

static QState Distance_top(Distance * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      Serial.println("Distance_top ENTRY");
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      /*Serial.println("check for detection...");*/

      auto range = getRange();

      // range is min: 10cm max: 80cm
      if (range >= 100 && range <= 800) {

        /*Serial.print(range);
        Serial.println(" mm");*/

        // object exists at 4-8" away
        if (range >= 100 && range <= 200) {
          me->current_range = range;
          return Q_TRAN(&Distance_detected);
        }
      }

      return Q_HANDLED();
    }
  }

  return Q_SUPER(&QHsm_top);
}

static QState Distance_detected(Distance * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      Serial.println("Distance_detected ENTRY");
      Serial.println("posting detection");
      QActive_post((QActive *) &AO_RC, OBJECT_DETECTED_SIG, me->current_range);
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      auto range = getRange();

      // range is min: 10cm max: 80cm
      if (range >= 100 && range <= 800) {
        // do nothing object still detected.
      } else {
        // clear
        Serial.println("object gone clearing");
        return Q_TRAN(&Distance_top);
      }

      return Q_HANDLED();
    }

    case Q_EXIT_SIG: {
      Serial.println("Distance_detected EXIT");
      Serial.println("clearing detection");
      QActive_post((QActive *) &AO_RC, OBJECT_CLEAR_SIG, me->current_range);
      return Q_HANDLED();
    }
  }

  return Q_SUPER(&Distance_top);
}