/*    
    Copyright 2020, Network Research Lab at the University of Toronto.

    This file is part of CottonCandy.

    CottonCandy is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CottonCandy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CottonCandy.  If not, see <https://www.gnu.org/licenses/>.
*/

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

// The time (milliseconds) between gateway requesting data
// Currently set to every 300 seconds (5 minutes)
#define GATEWAY_REQ_TIME 300000

LoRaMesh *manager;

DeviceDriver *myDriver;

// 2-byte long address 
// For Gateway only: The first bit of the address has to be 1
byte myAddr[2] = {0x80, 0xA0};

/**
 * Callback function that will be called when Gateway receives the reply from a node
 */
void onReciveResponse(byte *data, byte len)
{

  // In the example, we simply print out the reply message from the node
  Serial.print(F("Gateway received a node reply: "));

  for (int i = 0; i < len; i++)
  {
    Serial.print((data[i]), HEX);
  }
  Serial.println(".");
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
  // myDriver = new AdafruitDeviceDriver(myAddr, 0x09);

  // Initialize the driver
  myDriver->init();

  // Create a LoRaMesh object
  manager = new LoRaMesh(myAddr, myDriver);

  // For Gateway only: Set up the time interval between requests
  manager->setGatewayReqTime(GATEWAY_REQ_TIME);

  // Set up the callback funtion
  manager->onReceiveResponse(onReciveResponse);
}

void loop()
{
  Serial.println("Loop starts");

  // Start the gateway
  manager->run();
}
