//
// Created by yipstar on 2019-02-19.
//

#include <Arduino.h>
#include <Wire.h>
#include <ZumoShield.h>

#include "qpn.h"
#include "app.h"

#define RC_DEBUG 1
#include "log.h"

enum RC_Mode {
  RC_MODE_DISARM,
  RC_MODE_MANUAL,
  RC_MODE_AUTO
};

typedef struct RC {
    QActive super;
} RC;

// forward declare state handlers
static QState RC_initial(RC * const me);
static QState RC_top(RC * const me);

//static QState RC_manual(RC * const me);
static QState RC_manual_good(RC * const me);
static QState RC_manual_nogood(RC * const me);

static QState RC_auto(RC * const me);
static QState RC_auto_object_detected(RC * const me);

RC AO_RC;
void RC_ctor(void) {
    // do nothing
    RC *me = &AO_RC;
    QActive_ctor(&me->super, Q_STATE_CAST(&RC_initial));
}

// forward declare helper functions
void doManual(int throttle, int steering);

RC_Mode getMode();

// elaborate state machine
QState RC_initial(RC * const me) {

  // Arm RC input at 10hz
  QActive_armX((QActive *)me, 0U,
               BSP_TICKS_PER_SEC/10U, BSP_TICKS_PER_SEC/10U);

  return Q_TRAN(&RC_top);
}

RC_Mode getMode() {
  auto mode = RC_MODE_DISARM;

  int modeIn = pulseIn(MODE_PIN, HIGH);
  // RC signals encode information in pulse width centered on 1500 us (microseconds);
  // subtract 1500 to get a value centered on 0
  modeIn -= 1500;

  /*RC_PRINT("modeIn: ");
  RC_PRINTLN(modeIn);*/

  if (modeIn < -500) {
    mode = RC_MODE_MANUAL;
  }

  if (modeIn > 470) {
    mode = RC_MODE_AUTO;
  }

  RC_PRINT("mode ");
  RC_PRINTLN(mode);

  return mode;
}

static QState RC_top(RC * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      /*RC_PRINTLN("RC_top");*/
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      Serial.println("RC_top: timeout");

      auto mode = getMode();
      if (mode == RC_MODE_MANUAL) {
        /*return Q_TRAN(&RC_manual_nogood);*/
      }

      if (mode == RC_MODE_AUTO) {
        /*return Q_TRAN(&RC_auto);*/

      }

      return Q_HANDLED();
    }

    case OBJECT_DETECTED_SIG: {
      Serial.println("RC_top: object detected, do nothing");
      return Q_HANDLED();
    }

    case OBJECT_CLEAR_SIG: {
      Serial.println("RC_top: object clear, do nothing");
      return Q_HANDLED();
    }

  }

  return Q_SUPER(&QHsm_top);
}

static QState RC_manual_good(RC * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      RC_PRINTLN("RC_manual_good");
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      auto mode = getMode();
      if (mode != RC_MODE_MANUAL) {
        return Q_TRAN(&RC_top);
      }

      /*RC_PRINTLN("read data");*/

      int throttle = pulseIn(THROTTLE_PIN, HIGH);
      int steering = pulseIn(STEERING_PIN, HIGH);

      if (!(throttle > 0 && steering > 0)) {
        return Q_TRAN(&RC_manual_nogood);
      }

      doManual(throttle, steering);

      return Q_HANDLED();
    }

    case Q_EXIT_SIG: {
      RC_PRINTLN("RC_manual_good EXIT");
      ZumoMotors::setSpeeds(0, 0);
      return Q_HANDLED();
    }

  }

  return Q_SUPER(&RC_top);
}

static QState RC_manual_nogood(RC * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      RC_PRINTLN("RC_manual_nogood");
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {

      int throttle = pulseIn(THROTTLE_PIN, HIGH);
      int steering = pulseIn(STEERING_PIN, HIGH);

      if (throttle > 0 && steering > 0) {
        return Q_TRAN(&RC_manual_good);
      } else {
        return Q_HANDLED();
      }
    }

  }

  return Q_SUPER(&RC_top);
}

static QState RC_auto(RC * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      RC_PRINTLN("RC_auto");
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      /*RC_PRINLN("RC_auto: timeout");*/

      auto mode = getMode();
      if (mode != RC_MODE_AUTO) {
        return Q_TRAN(&RC_top);
      }

      int forwardSpeed = 80;
      ZumoMotors::setSpeeds(forwardSpeed, forwardSpeed);

      return Q_HANDLED();
    }

    case OBJECT_DETECTED_SIG: {
      Serial.println("RC_auto object_detected");
      return Q_TRAN(&RC_auto_object_detected);
    }

    case Q_EXIT_SIG: {
      RC_PRINTLN("RC_auto EXIT");
      ZumoMotors::setSpeeds(0, 0);
      return Q_HANDLED();
    }
  }

  return Q_SUPER(&RC_top);
}

static QState RC_auto_object_detected(RC * const me) {

  switch (Q_SIG(me)) {

    case Q_ENTRY_SIG: {
      RC_PRINTLN("RC_auto_object_detected ENTRY");
      return Q_HANDLED();
    }

    case Q_TIMEOUT_SIG: {
      RC_PRINTLN("RC_auto_object_detected timeout");

      ZumoMotors::setSpeeds(0, 0);
      return Q_HANDLED();
    }

    case OBJECT_DETECTED_SIG: {
      // do nothing with this message
      return Q_HANDLED();
    }

    case OBJECT_CLEAR_SIG: {
      Serial.println("RC_auto_object_detected: object clear");
      return Q_TRAN(&RC_auto);
    }
  }

  return Q_SUPER(&RC_auto);
}

void doManual(int throttle, int steering) {

  // RC signals encode information in pulse width centered on 1500 us (microseconds);
  // subtract 1500 to get a value centered on 0
  throttle -= 1500;
  steering -= 1500;

  // apply deadband
  if (abs(throttle) <= PULSE_WIDTH_DEADBAND)
    throttle = 0;
  if (abs(steering) <= PULSE_WIDTH_DEADBAND)
    steering = 0;

  RC_PRINT("throttle: ");
  RC_PRINTLN(throttle);
  RC_PRINT("steering: ");
  RC_PRINTLN(steering);

  // mix throttle and steering inputs to obtain left & right motor speeds
  // TODO: research this formula
  int left_speed = ((long)throttle * MAX_SPEED / PULSE_WIDTH_RANGE) - ((long)steering * MAX_SPEED / PULSE_WIDTH_RANGE);
  int right_speed = ((long)throttle * MAX_SPEED / PULSE_WIDTH_RANGE) + ((long)steering * MAX_SPEED / PULSE_WIDTH_RANGE);

  // cap speeds to max
  left_speed = min(max(left_speed, -MAX_SPEED), MAX_SPEED);
  right_speed = min(max(right_speed, -MAX_SPEED), MAX_SPEED);

  RC_PRINT("left_speed: ");
  RC_PRINTLN(left_speed);
  RC_PRINT("right_speed: ");
  RC_PRINTLN(right_speed);

  ZumoMotors::setSpeeds(right_speed, left_speed);
}