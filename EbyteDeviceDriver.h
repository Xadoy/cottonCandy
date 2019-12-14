#ifndef HEADER_EBYTE_DEVICE_DRIVER
#define HEADER_EBYTE_DEVICE_DRIVER

#include "DeviceDriver.h"
#include "SoftwareSerial.h"

#define BAUD_RATE 9600

typedef enum Mode
{
  TRANSMIT,
  CONFIG,
  WOR,
  SLEEP
};

class EbyteDeviceDriver : public DeviceDriver{
public:
    EbyteDeviceDriver(uint8_t rx, uint8_t tx, uint8_t m0, uint8_t m1, uint8_t aux_pin, address addr, uint8_t channel);

    ~EbyteDeviceDriver();

    bool init();

    int send(address destAddr, uint8_t channel, char* msg, long msgLen);

    char recv();

private:
    SoftwareSerial* module;
    uint8_t rx;
    uint8_t tx;
    uint8_t m0;
    uint8_t m1;
    uint8_t aux_pin;
    uint8_t currentMode = 0;

    address myAddr;
    uint8_t myChannel;

    /*-----------Module Registers Configuration-----------*/
    void setAddress(address addr);
    void setChannel(uint8_t channe);
    void setNetId(uint8_t netId);
    void setOthers(byte config);

    void enterConfigMode();
    void enterTransMode();
    void enterWorMode();
    void enterSleepMode();
    uint8_t getCurrentMode();

    /*-----------Helper Function-----------*/
    void receiveConfigReply(int replyLen);
};

#endif