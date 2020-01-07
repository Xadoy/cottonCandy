#include "AdafruitDeviceDriver.h"

#define DEBUG 1

/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915.0

AdafruitDeviceDriver::AdafruitDeviceDriver(byte* addr, 
                                    uint8_t channel) : DeviceDriver(){
    module = new RH_RF95(RFM95_CS, RFM95_INT);

    myAddr = addr;
    myChannel = channel;
}

AdafruitDeviceDriver::~AdafruitDeviceDriver(){
    delete module;
}

bool AdafruitDeviceDriver::init(){

    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    while (!module->init()) {
        Serial.println("LoRa radio init failed");
        return false;
    }

    if (!module->setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        return false;
    }
    module->setTxPower(23, false);

    Serial.println("LoRa Module initialized");

    setAddress(myAddr);
    setChannel(myChannel);
    return true;
}

int AdafruitDeviceDriver::send(byte* destAddr, byte* msg, long msgLen){
    // byte data[3 + msgLen];
    // memcpy(data, destAddr, 2);
    // data[2] = (byte)myChannel;
    
    // memcpy(data + 3, msg, msgLen);
    // Serial.print("Messsage to be sent 0x");

    // for(int i = 0; i < msgLen + 3; i++){
    //     Serial.print(data[i], HEX);
    //     Serial.print(" ");
    // }
    // Serial.print("\n");
    // return (module->write(data, sizeof(data)));
    Serial.print("bytes to send: ");
    for(int i = 0; i < msgLen; i++){
        Serial.print("0x");
        Serial.print(msg[i], HEX);
        Serial.print(" ");
    }
    Serial.println("");
    if (module->send(msg, (uint8_t)msgLen)) {
        Serial.println("msg sent");
        module->waitPacketSent();
        return (int)msgLen;
    }
    else {
        Serial.println("msg failed to send");
        return -1;
    }
    // return module->send(msg, (uint8_t)msgLen) ? (int)msgLen : -1;
}


byte AdafruitDeviceDriver::recv(){
    byte buf[1];
    uint8_t len = 1;
    module->waitPacketSent();
   if(module->waitAvailableTimeout(5000) && module->recv(buf, &len)){
        // Serial.print("number of bytes recv: ");
        // Serial.print(len);
        Serial.print("content: ");
        Serial.print(buf[0], HEX);
        Serial.println("");
        return buf[0];
   }
   else{
       Serial.println("failed to receive");
        return -1;
   }
}
int AdafruitDeviceDriver::available(){
    return module->available() ? 1 : 0;
}
int AdafruitDeviceDriver::getLastMessageRssi(){

    int result = module->lastRssi();
    return result;
}


uint8_t AdafruitDeviceDriver::getCurrentMode()
{
    return this->currentMode;
}

/*-----------LoRa Configuration-----------*/
void AdafruitDeviceDriver::setAddress(byte* addr)
{
    myAddr = addr;
}

void AdafruitDeviceDriver::setChannel(uint8_t channel)
{
    Serial.print("adafruit driver set Channel is not ready yet ");
    // Serial.print(channel);
    // Serial.print("\n");
}
