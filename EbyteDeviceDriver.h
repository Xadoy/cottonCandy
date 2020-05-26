#ifndef HEADER_EBYTE_DEVICE_DRIVER
#define HEADER_EBYTE_DEVICE_DRIVER

#include "Arduino.h"
#include "DeviceDriver.h"
#include "SoftwareSerial.h"

#define BAUD_RATE 9600
#define EBYTE_ADDRESS_SIZE 2

typedef enum 
{
  TRANSMIT,
  CONFIG,
  WOR,
  SLEEP
} Mode;

class EbyteDeviceDriver : public DeviceDriver{
public:
    EbyteDeviceDriver(uint8_t rx, uint8_t tx, uint8_t m0, uint8_t m1, uint8_t aux_pin, byte* addr, uint8_t channel);

    ~EbyteDeviceDriver();

    bool init();

    int send(byte* destAddr, byte* msg, long msgLen);

    byte recv();

    int available();

    int getLastMessageRssi();

private:
    SoftwareSerial* module;
    uint8_t rx;
    uint8_t tx;
    uint8_t m0;
    uint8_t m1;
    uint8_t aux_pin;
    uint8_t currentMode = 0;

    byte myAddr[2];
    uint8_t myChannel;

    /*-----------Module Registers Configuration-----------*/
    void setAddress(byte* addr);
    void setChannel(uint8_t channe);
    void setNetId(uint8_t netId);
    void setOthers(byte config);
    void setEnableRSSI();

    /**
     * The function sets the air rate to 9.6kbps by default
     */ 
    void setAirRate();

    void enterConfigMode();
    void enterTransMode();
    void enterWorMode();
    void enterSleepMode();
    uint8_t getCurrentMode();

    /*-----------Helper Function-----------*/
    void receiveConfigReply(int replyLen);
};

#endif
