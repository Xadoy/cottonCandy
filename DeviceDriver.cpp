#include "DeviceDriver.h"

DeviceDriver::DeviceDriver(){

}

DeviceDriver::~DeviceDriver(){

}

bool DeviceDriver::init(){
    return true;
}

int DeviceDriver::send(address destAddr, uint8_t channel, char* msg, long msgLen){
    Serial.println("Send not implemented in this dummy driver");
    return -1;
}

char DeviceDriver::recv(){
    Serial.println("Recv not implemented in this dummy driver");
    return -1;
}