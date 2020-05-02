# CottonCandy
CottonCandy is an Arduino software package for LoRa mesh networking. It provides a universal network layer for building multi-hop mesh networks on LoRa devices.

## Compatible Hardware
CottonCandy is designed to support virually any types of LoRa transceivers that has interface for the following software operations:
1. Transmit and receive a message
2. Read the RSSI value of a received message (for organzing network topology)

The cottonCandy library has been tested on the following types of hardware:
1. [Adafruit Feather 32u4](https://www.adafruit.com/product/3078)
2. [EByte E22 test board](http://www.ebyte.com/en/pdf-down.aspx?id=1039) 

**Note**: For transceivers that do not have an embedded microcontroller (e.g. EByte E22), you need to connect the transceiver hardware to a microcontroller board (e.g. Arduino Uno), so that the software can interface with the transceiver via software serial.

## Installation
### Using GitHub
```sh
cd ~/Documents/Arduino/libraries/
git clone https://github.com/infernoDison/cottonCandy.git
```

## Getting Started

### Set up Hardware Driver
To build mesh networks with cottonCandy, you need to provide an implementation to the `Device Driver` class based on your LoRa hardware. 

#### Adafruit and EByte Hardware
CottonCandy has prepared implementations of `Device Driver` for both Adafruit Feather 32u4 and EByte E22. Please feel free to change those driver implementations to suit your own needs.

**Note:** 
* The implementation for EByte test boards uses SoftwareSerial. The default RX and TX size in SoftwareSerial is 64 Bytes. We recommend you to increase the size for large networks, by changing `_SS_MAX_RX_BUFF` and `_SS_MAX_TX_BUFF` in the SoftwareSerial header file.
* The implementation for Adafruit Feather 32u4 requires the open-source library ["arduino-LoRa"](https://www.github.com/sandeepmistry/arduino-LoRa) from Sandeep Mistry. You can follow the installation guide on its Github page.

#### Other Hardware
For other brands of LoRa transceivers, you need to provide your own implementation of the hardware driver
for them. Implementations of the following functions in `Device Driver` are mandatory.

* `int DeviceDriver::Send(byte* destAddr, byte* msg, long msgLen)`
* `byte DeviceDriver::recv()`
* `int DeviceDriver::getLastMessageRssi()`
* `int DeviceDriver::available()`

You can also implement `bool DeviceDriver::init()` in `Device Driver` in case your LoRa transceiver requires some initialization (e.g. Set the frequency).

CottonCandy uses point-to-point communication and broadcast address. Most of the messages are sent using "unicast", as non-recevier nodes simply ignore the message at the driver level and avoid further processing. Some hardware devices like EByte E22 already provides such address filtering in the firmware-level. For other LoRa devices which do not come with address filtering, you need to add the address filtering feature in the implementation of the hardware driver. The easiest way to do so is to insert "destination address" in the beginning of the packet upon sending and process it upon receiving the packet. An example is done in the "AdafruitDeviceDriver" provided.

### Set up Node
After constructing the device driver, you can construct a `LoRaMesh` object and start running the device as a network node. 

For example, the Arduino program can look like this:

```cpp
DeviceDriver* myDriver;

//Node address is 2-byte long
//Gateway Node ONLY: First bit in the address is set to 1
//Non-gateway Nodes: First bit in the address is set to 0
//Here, we are creating a gateway node
byte myAddr[2] = {0x80, 0x01}

//Construct the Device Driver for EByte hardware
myDriver = new EbyteDeviceDriver(LORA_RX, LORA_TX, LORA_M0, LORA_M1, LORA_AUX, myAddr, 0x09);

//Initialize and configure the LoRa transceiver (e.g. Set the frequency)
myDriver->init();

//Construct the LoRaMesh object
LoRaMesh* myManager = new LoRaMesh(myAddr, myDriver);

//Gateway Node ONLY: set the time interval for requesting data (milliseconds)
manager->setGatewayReqTime(60000);

//All Nodes: set the callback function when data (gateway) or request (regular nodes) are received
manager->onReveiveResponse(myCallback);

//Start the node
myManager->run();
```

## Network Topology and Protocol
Detailed design of the network protocol can be found in the [Wiki](https://github.com/infernoDison/cottonCandy/wiki)

## Authors
* **Dixin Wu**
* **Hongyi Yang**
* **Yizhi Xu**
