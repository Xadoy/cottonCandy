#include "Utilities.h"
#include "Arduino.h"


unsigned long getTimeMillis(){
    return millis();
}

void sleepForMillis(unsigned long time){
    delay(time);
}