#include <LoRaMesh.h>
//#include <EbyteDeviceDriver.h>
#include <AdafruitDeviceDriver.h>

#define LORA_RX 10
#define LORA_TX 11

#define LORA_M0 2
#define LORA_M1 3
#define LORA_AUX 4

LoRaMesh* manager;
DeviceDriver* myDriver;
byte myAddr[2] = {0x80, 0xA0};

void onReciveResponse(byte *data, byte len) {
  Serial.print("\nGateway received node reply from ");
  Serial.print((data[6]),HEX);
  Serial.print((data[7]),HEX);
  Serial.print(": ");
  for(int i = 0; i < len; i++){
      Serial.print((data[i]),HEX);
  }
  Serial.println(".");
}


void setup() {
    Serial.begin(57600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
//     myDriver = new EbyteDeviceDriver(LORA_RX, LORA_TX, LORA_M0, LORA_M1, LORA_AUX, myAddr, 0x09);
    myDriver = new AdafruitDeviceDriver(myAddr, 0x09);
    myDriver->init();
    
    manager = new LoRaMesh(myAddr,myDriver);
    manager->setGatewayReqTime(61000);
    manager->onReceiveResponse(onReciveResponse);
    
    Serial.println("Free memory left: ");
    Serial.println(freeMemory());
}

void loop() {
  
  Serial.println("Loop starts");
  manager->run();

}
