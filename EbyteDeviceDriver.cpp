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

#include "EbyteDeviceDriver.h"

#define DEBUG 1

EbyteDeviceDriver::EbyteDeviceDriver(uint8_t rx, uint8_t tx, uint8_t m0, uint8_t m1, uint8_t aux_pin, byte* addr, 
                                    uint8_t channel) : DeviceDriver(){
    this->rx = rx;
    this->tx = tx;
    this->m0 = m0;
    this->m1 = m1;
    this->aux_pin = aux_pin;
    module = new SoftwareSerial(rx, tx);

    if(sizeof(addr) < EBYTE_ADDRESS_SIZE){
        Serial.println("Error: Node address must be 2-byte long");
    }else{
        myAddr[0] = addr[0];
        myAddr[1] = addr[1];
    }

    myChannel = channel;
}

EbyteDeviceDriver::~EbyteDeviceDriver(){
    delete module;
}

bool EbyteDeviceDriver::init(){

    pinMode(this->m0, OUTPUT);
    pinMode(this->m1, OUTPUT);
    pinMode(this->aux_pin, INPUT);
    Serial.println(F("LoRa Module Pins initialized"));

    while (digitalRead(this->aux_pin) != HIGH)
    {
        Serial.println(F("Waiting for LoRa Module to initialize"));
        delay(10);
    }

    module->begin(BAUD_RATE);
    Serial.println(F("LoRa Module initialized"));

    enterConfigMode();
    setAddress(myAddr);
    setChannel(myChannel);
    setNetId(0x00);
    setAirRate();

    //6th Byte: 0101 0000
    //Fixed-Point tranmission: enabled
    //Listen-before-talk: enabled
    setOthers(0x50);


    setEnableRSSI();

    enterTransMode();
    Serial.println("Enter Transmission Mode");
    return true;
}
/**
 * Here we are using the fixed transmission feature in Ebyte. Thus, for every outgoing message, we
 * need to append a header indicating the destination address and channel.
 * 
 * The reason to use fixed transmission rather than transparent transmission is that fixed transmission
 * enables hardware address filtering in the Ebyte transceiver. Also broadcast can easily be done by
 * setting dest address to FFFF;
 */ 
int EbyteDeviceDriver::send(byte* destAddr, byte* msg, long msgLen){

    long totalLen = 3 + msgLen;

    byte* data = new byte[totalLen];
    memcpy(data, destAddr, EBYTE_ADDRESS_SIZE);
    data[2] = (byte)myChannel;
    
    memcpy(data + 3, msg, msgLen);
    /*
    Serial.print(F("Messsage to be sent 0x"));

    for(int i = 0; i < totalLen; i++){
         Serial.print(data[i], HEX);
        Serial.print(" ");
     }
    Serial.print("\n");
    */
    int bytesSent = module->write(data, totalLen);

    //Wait for the message written to the Ebyte chip
    //This delay is estimated by using the maximum message length (~70 Bytes) divided by the
    //baud rate (9600 8N1 ~= 1040 bytes per second) and get ~60ms
    delay(60);

    //Wait till Ebyte has transmitted the message
    while (digitalRead(this->aux_pin) != HIGH)
    {
    }

    delete[] data;

    return bytesSent;
}


byte EbyteDeviceDriver::recv(){
   if(module->available()){
       return (module->read());
   }
   else{
       return -1;
   }
}
int EbyteDeviceDriver::available(){
    return module->available();
}
int EbyteDeviceDriver::getLastMessageRssi(){

    // retrieve rssi from register
    module->write(0xC0);
    module->write(0xC1);
    module->write(0xC2);
    module->write(0xC3);

    module->write(0x01);
    module->write(0x01);

    int bytesRead = 0;
    int result = 0;
    while (bytesRead < 4)
    {
      if (module->available())
      {
        char b = module->read();
        if (bytesRead == 2)
            result = (int)b;
        bytesRead++;
      }
    }
    return result;
}

void EbyteDeviceDriver::enterConfigMode()
{
    digitalWrite(this->m0, LOW);
    digitalWrite(this->m1, HIGH);
    //Need to wait for the configuration to be in effect
    delay(1000);
    //Make Sure the AUX is now in HIGH state
    while (digitalRead(this->aux_pin) != HIGH)
    {
    }
    Serial.println(F("Successfully entered CONFIGURATION mode"));
    currentMode = Mode::CONFIG;
}

void EbyteDeviceDriver::enterTransMode()
{
    digitalWrite(this->m0, LOW);
    digitalWrite(this->m1, LOW);
    //Need to wait for the configuration to be in effect
    delay(1000);

    //Make Sure the AUX is now in HIGH state
    while (digitalRead(this->aux_pin) != HIGH)
    {
    }
    Serial.println(F("Successfully entered TRANSMISSION mode"));
    currentMode = Mode::TRANSMIT;
}

void EbyteDeviceDriver::enterWorMode()
{
    digitalWrite(this->m0, HIGH);
    digitalWrite(this->m1, LOW);

    //Need to wait for the configuration to be in effect
    delay(1000);
    //Make Sure the AUX is now in HIGH state
    while (digitalRead(this->aux_pin) != HIGH)
    {
    }
    Serial.println(F("Successfully entered WOR mode"));
    currentMode = Mode::WOR;
}

void EbyteDeviceDriver::enterSleepMode()
{
    digitalWrite(this->m0, HIGH);
    digitalWrite(this->m1, HIGH);

    //Need to wait for the configuration to be in effect
    delay(1000);
    //Make Sure the AUX is now in HIGH state
    while (digitalRead(this->aux_pin) != HIGH)
    {
    }
    Serial.println(F("Successfully entered SLEEP mode"));
    currentMode = Mode::SLEEP;
}

uint8_t EbyteDeviceDriver::getCurrentMode()
{
    return this->currentMode;
}

/*-----------LoRa Configuration-----------*/
void EbyteDeviceDriver::setAddress(byte* addr)
{
    module->write(0xC0);
    module->write((byte)0x00);
    module->write(0x02);
    module->write(addr, EBYTE_ADDRESS_SIZE);

    //Block and read the reply to clear the buffer
    receiveConfigReply(5);
    
    if(DEBUG){
        Serial.print(F("Successfully set Address to 0x"));
        Serial.print(addr[0], HEX);
        Serial.print(addr[1], HEX);
        Serial.print("\n");   
    }
}

void EbyteDeviceDriver::setNetId(uint8_t netId)
{
    module->write(0xC0);
    module->write(0x02);
    module->write(0x01);
    module->write((byte)netId);

    //Read the reply to clear the buffer
    receiveConfigReply(4);

    if(DEBUG){
        Serial.print(F("Successfully set Net Id to "));
        Serial.print(netId);
        Serial.print("\n");
    }
}

void EbyteDeviceDriver::setChannel(uint8_t channel)
{
    module->write(0xC0);
    module->write(0x05);
    module->write(0x01);
    module->write((byte)channel);

    //Read the reply to clear the buffer
    receiveConfigReply(4);

    if(DEBUG){
        //Frequency = 410.125 MHz + channel * 1 MHz
        Serial.print(F("Successfully set Channel to "));
        Serial.print(channel);
        Serial.print("\n");
    }
}

void EbyteDeviceDriver::setOthers(byte config)
{
    module->write(0xC0);
    module->write(0x06);
    module->write(0x01);
    module->write((byte)config);

    //Read the reply to clear the buffer
    receiveConfigReply(4);
    if(DEBUG){
        Serial.println(F("Successfully set other configs"));
    }
}

void EbyteDeviceDriver::receiveConfigReply(int replyLen)
{
    int bytesRead = 0;
    while (bytesRead < replyLen)
    {
        if (module->available())
        {
        byte b = module->read();
        //Serial.print(b,HEX);
        bytesRead++;
        }
    }
    //Serial.print("\n");
}

void EbyteDeviceDriver::setEnableRSSI()
{
  module->write(0xC0);
  module->write(0x04);
  module->write(0x01);
  module->write(0x20);

  //Read the reply to clear the buffer
  receiveConfigReply(4);
  if(DEBUG){
    Serial.println(F("Successfully enable RSSI"));
  }
}

void EbyteDeviceDriver::setAirRate(){

    //Set baud rate to 9600
    //Serial mode = 8N1
    //Air rate = 9.6kbps
    module->write(0xC0);
    module->write(0x03);
    module->write(0x01);
    //0x64 = 0b01100100
    module->write(0x64);

    //Read the reply to clear the buffer
    receiveConfigReply(4);
    if(DEBUG){
        Serial.println(F("Successfully set the air rate"));
    }
}
