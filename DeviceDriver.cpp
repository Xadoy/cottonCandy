#include "DeviceDriver.h"

DeviceDriver::DeviceDriver(){

}

DeviceDriver::~DeviceDriver(){

}

bool DeviceDriver::init(){
    return true;
}

int DeviceDriver::send(byte* destAddr, byte* msg, long msgLen){
    Serial.println("Send not implemented in this dummy driver");
    return -1;
}

byte DeviceDriver::recv(){
    Serial.println("Recv not implemented in this dummy driver");
    return -1;
}
