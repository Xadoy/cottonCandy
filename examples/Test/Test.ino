#include <ForwardEngine.h>
#include <EbyteDeviceDriver.h>

#define LORA_RX 10
#define LORA_TX 11

#define LORA_M0 2
#define LORA_M1 3
#define LORA_AUX 4

ForwardEngine* myEngine;
DeviceDriver* myDriver;
address myAddr = 1;

void setup() {
    myDriver = new EbyteDeviceDriver(LORA_RX, LORA_TX, LORA_M0, LORA_M1, LORA_AUX, myAddr, (uint8_t)9);
    //myEngine = new ForwardEngine(myAddr,myDriver);
    myDriver->init();

    
    //delete myEngine;
}

void loop() {
  // put your main code here, to run repeatedly:
  myDriver->send(0x5678, "Hello World", 11);
  delay(3000);

}
