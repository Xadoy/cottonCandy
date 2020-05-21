#include "AdafruitDeviceDriver.h"
#include <SPI.h>
#include <LoRa.h>

#define DEBUG 1

byte msgQueue[MSG_QUEUE_CAPACITY];
uint8_t queueHead; //next index to read;
uint8_t queueTail; //next index to write
uint8_t queueSize;

byte *adafruitAddr;

AdafruitDeviceDriver::AdafruitDeviceDriver(byte *addr,
                                           long frequency, int sf, long bw, int cr) : DeviceDriver()
{

    // Assign class member variables
    setAddress(addr);
    setFrequency(frequency);
    setSpreadingFactor(sf);
    setChannelBandwidth(bw);
    setCodingRateDenominator(cr);

    queueHead = 0;
    queueTail = 0;
    queueSize = 0;
}

AdafruitDeviceDriver::~AdafruitDeviceDriver()
{
}

void onReceive(int packetSize)
{
    packetSize -= 2;
    if (queueSize + packetSize > MSG_QUEUE_CAPACITY)
    {
        Serial.println(F("Warning, packet is dropped since buffer is full"));
        return;
    }
    byte add0 = LoRa.read();
    byte add1 = LoRa.read();

    if ((adafruitAddr[0] != add0 ||
         adafruitAddr[1] != add1) &&
        !(add0 == 0xFF && add1 == 0xFF))
    {
        // Serial.println("Addr unmatched, packet is dropped");
        return;
    }
    // Serial.print("onReceive to queue, size: ");
    // Serial.println(packetSize);
    // Serial.print("Before, queue size: ");
    // Serial.println(queueSize);
    while (LoRa.available())
    {
        byte result = LoRa.read();
        // Serial.print("0x");
        // Serial.print(result,HEX);
        // Serial.print(" ");
        queueSize++;
        msgQueue[queueTail] = result;
        queueTail = (queueTail + 1) % MSG_QUEUE_CAPACITY;
    }
    //     Serial.println("");
    //     Serial.print("After, queue end: ");
    //     Serial.println(queueSize);
}
bool AdafruitDeviceDriver::init()
{

    LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);

    if (!LoRa.begin(freq))
    {
        Serial.println(F("Starting LoRa failed!"));
        return false;
    }

    // Assign the parameters to the actual LoRa module which writes to the hardware register
    LoRa.setSpreadingFactor(sf);
    LoRa.setSignalBandwidth(channelBW);
    LoRa.setCodingRate4(codingRate);
    // LoRa.setTxPower(23);

    LoRa.onReceive(onReceive);
    LoRa.receive();
    Serial.println(F("LoRa Module initialized"));

    return true;
}

int AdafruitDeviceDriver::send(byte *destAddr, byte *msg, long msgLen)
{
    byte data[2 + msgLen];
    memcpy(data, destAddr, 2);
    // data[2] = (byte)myChannel;

    memcpy(data + 2, msg, msgLen);

    LoRa.beginPacket();
    LoRa.write(data, (size_t)(msgLen + 2));
    int result = LoRa.endPacket(false) == 1 ? 1 : -1;
    LoRa.receive();
    return result;
}

byte AdafruitDeviceDriver::recv()
{
    if (available())
    {
        byte result = msgQueue[queueHead];
        //Serial.print("Received: 0x");
        //Serial.print(result,HEX);
        //Serial.println("");
        queueHead = (queueHead + 1) % MSG_QUEUE_CAPACITY;
        queueSize--;
        return result;
    }
    else
    {
        return -1;
    }
}
int AdafruitDeviceDriver::available()
{
    return queueSize;
}
int AdafruitDeviceDriver::getLastMessageRssi()
{

    int result = LoRa.packetRssi();
    return result;
}

/*-----------LoRa Configuration-----------*/
void AdafruitDeviceDriver::setAddress(byte *addr)
{

    if (sizeof(addr) < 2)
    {
        Serial.println("Error: Node address must be 2-byte long");
    }
    else
    {
        this->addr[0] = addr[0];
        this->addr[1] = addr[1];
        adafruitAddr = this->addr;
    }
}

void AdafruitDeviceDriver::setFrequency(long frequency)
{
    this->freq = frequency;
}

void AdafruitDeviceDriver::setSpreadingFactor(int sf)
{
    this->sf = sf;
}

void AdafruitDeviceDriver::setChannelBandwidth(long bw)
{
    this->channelBW = bw;
}

void AdafruitDeviceDriver::setCodingRateDenominator(int cr)
{
    this->codingRate = cr;
}