//TODO: This is currently a dummy implementation of the message processor
#ifndef HEADER_MESSAGE_PROCESSOR
#define HEADER_MESSAGE_PROCESSOR

#include "DeviceDriver.h"

typedef unsigned short address;

void sendBeacon();

void sendBeaconACK(address destAddr);

void sendBeaconCFM(address destAddr);

/* We will use polymorphism here */
class GenericMessage{

public:
    unsigned char type;
    address addr;

};

class Beacon : public GenericMessage{

};

GenericMessage* receiveMessage();

#endif