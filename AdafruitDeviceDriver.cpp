#include "AdafruitDeviceDriver.h"
#include <SPI.h>
#include <LoRa.h>

#define DEBUG 1

/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915E6

#define MSG_QUEUE_CAPACITY 255
byte msgQueue[MSG_QUEUE_CAPACITY];
uint8_t queueHead; //next index to read;
uint8_t queueTail; //next index to write
uint8_t queueSize;

byte *adafruitAddr;

AdafruitDeviceDriver::AdafruitDeviceDriver(byte* addr, 
                                    uint8_t channel) : DeviceDriver(){
    adafruitAddr = addr;
    myChannel = channel;
    queueHead = 0;
    queueTail = 0;
    queueSize = 0;
}

AdafruitDeviceDriver::~AdafruitDeviceDriver(){

}

void onReceive(int packetSize) {
    packetSize -= 2;
    if (queueSize + packetSize > MSG_QUEUE_CAPACITY) {
        Serial.println("Warning, packet is dropped since buffer is full");
        return;
    }
    byte add0 = LoRa.read();
    byte add1 = LoRa.read();
    if (adafruitAddr[0] & add0 != adafruitAddr[0] ||
        adafruitAddr[1] & add1 != adafruitAddr[1] ) {
        Serial.println("Addr unmatched, packet is dropped");
        return;
    }
    Serial.print("onReceive to queue, size: ");
    Serial.println(packetSize);
    Serial.print("Before, queue size: ");
    Serial.println(queueSize);
    while(LoRa.available()) {
        byte result = LoRa.read();
        Serial.print("0x");
        Serial.print(result,HEX);
        Serial.print(" ");
        queueSize++;
        msgQueue[queueTail] = result;
        queueTail = (queueTail + 1) % MSG_QUEUE_CAPACITY;
    }
    Serial.println("");
    Serial.print("After, queue end: ");
    Serial.println(queueSize);
}
bool AdafruitDeviceDriver::init(){

    LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
    // LoRa.setTxPower(23);
    if (!LoRa.begin(RF95_FREQ)) {
        Serial.println("Starting LoRa failed!");
        return false;
    }
    LoRa.onReceive(onReceive);
    LoRa.receive();
    Serial.println("LoRa Module initialized");

    setAddress(adafruitAddr);
    setChannel(myChannel);
    return true;
}

int AdafruitDeviceDriver::send(byte* destAddr, byte* msg, long msgLen){
    byte data[2 + msgLen];
    memcpy(data, destAddr, 2);
    // data[2] = (byte)myChannel;
    
    memcpy(data + 2, msg, msgLen);
    // Serial.print("Messsage to be sent 0x");

    // for(int i = 0; i < msgLen + 3; i++){
    //     Serial.print(data[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.print("\n");
    // return (module->write(data, sizeof(data)));
    Serial.print("bytes to send: ");
    for(int i = 0; i < msgLen + 2; i++){
        Serial.print("0x");
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println("");
    // if (module->send(msg, (uint8_t)msgLen)) {
    //     Serial.println("msg sent");
    //     module->waitPacketSent();
    //     return (int)msgLen;
    // }
    // else {
    //     Serial.println("msg failed to send");
    //     return -1;
    // }
    LoRa.beginPacket();
    LoRa.write(data, (size_t)(msgLen + 2));
    int result = LoRa.endPacket(false) == 1 ? 1 : -1;
    LoRa.receive();
    return result;
    // return module->send(msg, (uint8_t)msgLen) ? (int)msgLen : -1;
}


byte AdafruitDeviceDriver::recv(){
    if(available()) {
        byte result = msgQueue[queueHead];
        // Serial.print("Received: 0x");
        // Serial.print(result,HEX);
        // Serial.println("");
        queueHead = (queueHead + 1) % MSG_QUEUE_CAPACITY;
        queueSize--;
        return result;
    }
    else {
        return -1;
    }
}
int AdafruitDeviceDriver::available(){
    return queueSize;
}
int AdafruitDeviceDriver::getLastMessageRssi(){

    int result = LoRa.packetRssi();
    return result;
}


uint8_t AdafruitDeviceDriver::getCurrentMode()
{
    return this->currentMode;
}

/*-----------LoRa Configuration-----------*/
void AdafruitDeviceDriver::setAddress(byte* addr)
{
    adafruitAddr = addr;
}

void AdafruitDeviceDriver::setChannel(uint8_t channel)
{
    Serial.print("adafruit driver set Channel is not ready yet ");
    // Serial.print(channel);
    // Serial.print("\n");
}
