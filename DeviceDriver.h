//TODO: This is currently a dummy implementation for the driver class

#ifndef HEADER_DEVICE_DRIVER
#define HEADER_DEVICE_DRIVER

#include "Arduino.h"

#define BROADCAST_ADDR 0xFFFF

typedef unsigned short address;

class DeviceDriver{

public:
    //Default Constructor
    DeviceDriver();

    //Destructor
    virtual ~DeviceDriver();

    /**
     * Initialize the driver by configuring it to the correct mode, frequency etc.
     */
    virtual bool init();

    /**
     * Send a stream of bytes. Takes a byte array and its length.
     * Returns number of bytes successfully sent. Returns -1 if sending failed (timeout).
     */
    virtual int send(address destAddr, char* msg, long msgLen) = 0;

    /**
     * Returns a byte received. Returns -1 if none available
     */
    virtual char recv() = 0;

    virtual int getLastMessageRssi() = 0;

private:
    /**
     * Default receive timeout is 5000 ms (5 seconds) 
     */
    unsigned int receiveTimeOut = 5000;
};

#endif
