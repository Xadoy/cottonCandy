#include <LoRaMesh.h>
#include <EbyteDeviceDriver.h>
// Uncomment the next line for using Adafruit LoRa Feather
// #include <AdafruitDeviceDriver.h>

/**
 * Define pins for connection with Ebyte LoRa transceiver
 */
#define LORA_RX 10
#define LORA_TX 11
#define LORA_M0 2
#define LORA_M1 3
#define LORA_AUX 4

LoRaMesh *manager;

DeviceDriver *myDriver;

// 2-byte long address
// For regular nodes: The first bit of the address is 0
byte myAddr[2] = {0x00, 0xA0};

/**
 * Callback function that will be called when the node receives the request from the gateway
 * and needs to reply back
 */
void onReceiveRequest(byte **data, byte *len)
{

  // In the example, we simply send a series of HEX number of "ABCDEF" followed with the least significant byte of the node address
  Serial.println("onReciveRequest callback");
  (*data)[0] = 0xA;
  (*data)[1] = 0xB;
  (*data)[2] = 0xC;
  (*data)[3] = 0xD;
  (*data)[4] = 0xE;
  (*data)[5] = 0xF;
  (*data)[6] = 0xA;
  (*data)[7] = 0x0;
  *len = 0x08;
  for (int i = 0; i < *len; i++)
  {
    Serial.print(((*data)[i]), HEX);
  }
  Serial.print('\n');
}

void setup()
{
  Serial.begin(57600);

  // Wait for serial port to connect.
  while (!Serial)
  {
    ;
  }

  // In this example, we will be using an EByte E22 LoRa transceiver
  myDriver = new EbyteDeviceDriver(LORA_RX, LORA_TX, LORA_M0, LORA_M1, LORA_AUX, myAddr, 0x09);

  // Uncomment the next line for using Adafruit LoRa Feather
  // myDriver = new AdafruitDeviceDriver(myAddr);
  myDriver->init();

  // Create a LoRaMesh object
  manager = new LoRaMesh(myAddr, myDriver);

  // Set up the callback funtion
  manager->onReceiveRequest(onReceiveRequest);
}

void loop()
{
  Serial.println("Loop starts");

  // Start the node
  manager->run();
}
