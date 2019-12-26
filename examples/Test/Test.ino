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
    Serial.begin(57600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
    myDriver = new EbyteDeviceDriver(LORA_RX, LORA_TX, LORA_M0, LORA_M1, LORA_AUX, myAddr, 9);
    myEngine = new ForwardEngine(myAddr,myDriver);
    myDriver->init();

    Serial.println("Free memory left: ");
    Serial.println(freeMemory());
    delete myEngine;
}

void loop() {
  // put your main code here, to run repeatedly:
  myDriver->send(0x5678, "Hello World", 11);
  Serial.println("Free memory left: ");
  Serial.println(freeMemory());
  delay(3000);

}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}