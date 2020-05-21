#ifndef HEADER_ADAFRUIT_DEVICE_DRIVER
#define HEADER_ADAFRUIT_DEVICE_DRIVER

#include "Arduino.h"
#include "DeviceDriver.h"
#include "SoftwareSerial.h"

/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915E6

#define MSG_QUEUE_CAPACITY 255

#define DEFAULT_SPREADING_FACTOR 7
#define DEFAULT_CHANNEL_BW 125E3
#define DEFAULT_CODING_RATE_DENOMINATOR 5
typedef enum
{
  TRANSMIT,
  CONFIG,
  WOR,
  SLEEP
} Mode;

class AdafruitDeviceDriver : public DeviceDriver
{
public:
  AdafruitDeviceDriver(byte *addr, long frequency = RF95_FREQ, int sf = DEFAULT_SPREADING_FACTOR, long bw = DEFAULT_CHANNEL_BW, int cr = DEFAULT_CODING_RATE_DENOMINATOR);

  ~AdafruitDeviceDriver();

  bool init();

  int send(byte *destAddr, byte *msg, long msgLen);

  byte recv();

  int available();

  int getLastMessageRssi();

private:
  byte addr[2];
  long freq;
  int sf;
  long channelBW;
  int codingRate;

  /*-----------Module Registers Configuration-----------*/
  void setAddress(byte *addr);
  void setFrequency(long frequency);
  void setSpreadingFactor(int sf);
  void setChannelBandwidth(long bw);
  void setCodingRateDenominator(int cr);
};

#endif
