#include "Arduino.h"
#include "qpn.h"
#include "bsp.h"

#include "app.h"



static QEvt l_vehicleQueue[10]; // Event queue storage for Vehicle
static QEvt l_rcQueue[10]; // Event queue storage for RC

// QF_active[] array defines all active object control blocks ----------------
QActiveCB const Q_ROM QF_active[] = {
                                     { (QActive *)0, (QEvt *)0, 0U },
                                     { (QActive *)&AO_Vehicle, l_vehicleQueue, Q_DIM(l_vehicleQueue) },
                                     { (QActive *)&AO_RC, l_rcQueue, Q_DIM(l_rcQueue) }
};

void setup() {
    Serial.begin(9600);

    // initialize the QF-nano framework
    QF_init(Q_DIM(QF_active));

    // initialize all AOs...
    Vehicle_ctor();
    RC_ctor();

    // initialize the hardware used in this sketch...
    // set the LED-L pin to output
    pinMode(LED_L, OUTPUT);
}

void loop() {

    /*Serial.println("loop test 67");*/

    // run the QF-nano framework
    QF_run();
}
