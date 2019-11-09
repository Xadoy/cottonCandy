#ifndef HEADER_FORWARD_ENGINE
#define HEADER_FORWARD_ENGINE

#include "DeviceDriver.h"
#include "MessageProcessor.h"
#include "Utilities.h"

#define INIT 0
#define SEARCH 1
#define SELECT 2
#define CREATE 3

/* Setting the first bit of the address to 0 indicates a gateways. */
#define GATEWAY_ADDRESS_MASK 0x8000

/* The default time for discovery is 30 seconds */
#define DISCOVERY_TIME 30000

class ForwardEngine{

public:
    //Default Constructor
    ForwardEngine();

    //Copy constructor
    ForwardEngine(const ForwardEngine &node);

    //Destructor
    ~ForwardEngine();

    //Constructor. Requires driver and assigned addr
    //TODO: Uncomment when driver has been done
    ForwardEngine(DeviceDriver driver);

    int join();

    int disconnect();

    //Setter for the node address
    void setAddr(short addr);



private:
    //Node address
    address myAddr;

    //DeviceDriver driver;
    //driver

    //Parent address
    address parentAddr;

    //Current State
    char state;

    GenericMessage* discovery();

    bool pickParent(address pAddr1, address pAddr2);

};

#endif