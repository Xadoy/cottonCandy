#ifndef HEADER_ADAFRUIT_DEVICE_DRIVER
#define HEADER_ADAFRUIT_DEVICE_DRIVER

#include "Arduino.h"
#include "DeviceDriver.h"
#include "SoftwareSerial.h"
#include <SPI.h>
#include <RH_RF95.h>


typedef enum 
{
  TRANSMIT,
  CONFIG,
  WOR,
  SLEEP
} Mode;

class AdafruitDeviceDriver : public DeviceDriver{
public:
    AdafruitDeviceDriver(byte* addr, uint8_t channel);

    ~AdafruitDeviceDriver();

    bool init();

    int send(byte* destAddr, byte* msg, long msgLen);

    byte recv();

    int available();

    int getLastMessageRssi();

private:
    RH_RF95 *module;
    uint8_t currentMode = 0;

    byte* myAddr;
    uint8_t myChannel;

    /*-----------Module Registers Configuration-----------*/
    void setAddress(byte* addr);
    void setChannel(uint8_t channe);
    void setNetId(uint8_t netId);
    void setOthers(byte config);

    uint8_t getCurrentMode();

    /*-----------Helper Function-----------*/
    void receiveConfigReply(int replyLen);
};

#endif
