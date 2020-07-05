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

LoRaMesh *manager;

DeviceDriver *myDriver;

// 2-byte long address
// For regular nodes: The first bit of the address is 0
byte myAddr[2] = {0x00, 0xA0};

/**
 * In this example, we will use union to encode and decode the long-type integer we are sending
 * in the network
 */
union LongToBytes{
  long l;
  byte b[sizeof(long)];
};

/**
 * Callback function that is called when the node receives the request from the gateway
 * and needs to reply back. Users can read sensor value, do some processing and send data back
 * to the gateway.
 * 
 * "data" points to the payload portion of the reply packet that will be sent to the gateway
 * once the function returns. Users can write their own sensor value into the "data" byte array.
 * The length of the payload can be specified by writting it to "len"
 */
void onReceiveRequest(byte **data, byte *len)
{

  // In the example, we simply send a random integer value to the gateway
  Serial.println("onReciveRequest callback");

  // Generate a random value from 0 to 1000
  // In practice, this should be a sensor reading
  long sensorValue = random(0,1000);

  Serial.print(F("Sending number = "));
  Serial.print(sensorValue);
  Serial.println(F(" to the gateway"));

  // Specify the length of the payload
  *len = sizeof(long);

  // Encode this long-type value into a 4-byte array
  // Note: The encoding here using C++ union is little-endian. Although the common practice in networking
  // is big-endian, to make things simple, we uses the same union in both the sender(Node) and receiver(Gateway)
  // such that the number can be decoded correctly.
  union LongToBytes myConverter;
  myConverter.l = sensorValue;

  // Copy the encoded 4-byte array into the data (aka payload)
  memcpy(*data, myConverter.b, *len);
}

void setup()
{
  Serial.begin(57600);

  // Wait for serial port to connect.
  // Please comment out this while loop when using Adafruit feather or other ATmega32u4 boards 
  // if you do not want to connect Adafruit feather to a USB port for debugging. Otherwise, the
  // feather board does not start until a serial monitor is opened.
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
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
