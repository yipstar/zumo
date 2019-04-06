#include "Arduino.h"

#include "qpn.h"
#include "bsp.h"
#include "app.h"

static QEvt l_vehicleQueue[10]; // Event queue storage for Vehicle
static QEvt l_rcQueue[10]; // Event queue storage for RC
static QEvt l_distQueue[10]; // Event queue storage for distance sensor

// QF_active[] array defines all active object control blocks ----------------
// this order of active objects here defines the priorities.
QActiveCB const Q_ROM QF_active[] = {
                                     { (QActive *)0, (QEvt *)0, 0U },
                                     { (QActive *)&AO_RC, l_rcQueue, Q_DIM(l_rcQueue) },
                                     { (QActive *)&AO_Distance, l_distQueue, Q_DIM(l_distQueue) },
                                     { (QActive *)&AO_Vehicle, l_vehicleQueue, Q_DIM(l_vehicleQueue) }
};

void setup() {
  Serial.begin(9600);

  // initialize the QF-nano framework
  QF_init(Q_DIM(QF_active));

  // initialize all AOs...
  Vehicle_ctor();
  RC_ctor();
  Distance_ctor();

  // initialize the hardware used in this sketch...
  pinMode(LED_L, OUTPUT);
  pinMode(DIST_PIN, INPUT);
}

void loop() {
  // run the QF-nano framework
  QF_run();
}
