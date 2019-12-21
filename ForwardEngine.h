#ifndef HEADER_FORWARD_ENGINE
#define HEADER_FORWARD_ENGINE

#include "DeviceDriver.h"
#include "MessageProcessor.h"
#include "Utilities.h"

/*-------------States of a Node------------*/
#define INIT 0
#define SEARCH 1
#define SELECT 2
#define CREATE 3
#define JOINED 4

/* Setting the first bit of the address to 0 indicates a gateways. */
#define GATEWAY_ADDRESS_MASK 0x8000

/* The default time for discovery is 10 seconds */
#define DISCOVERY_TIMEOUT 10000

class ForwardEngine{

public:
    /**
     * Default Constructor
     */
    ForwardEngine();

    /**
     * Copy constructor
     */
    ForwardEngine(const ForwardEngine &node);

    /**
     * Destructor
     */
    ~ForwardEngine();

    /**
     * Constructor. Requires driver and assigned addr
     */
    ForwardEngine(address addr, DeviceDriver* driver);


    /**
     * Try to join an existing network by finding a parent. Return true if successfully joined an 
     * existing network
     */
    bool join();

    /**
     * Node exits an existing network
     */
    void disconnect();

    /**
     * This is the core loop where the node operates sending and receiving after it  joined the network.
     * Returns true if the node is started successfully.
     * 
     * Note: This method can be used without calling join() prior. In this case, the node assumes that
     * there is no existing network and it is the first node in the new network. 
     */
    bool run();

    int send();

    int receive();

    //Setter for the node address
    void setAddr(address addr);

    /**
     * Getter for the node address
     */
    address getMyAddr();

    /**
     * Getter for the parent address
     */
    address getParentAddr();

private:
    /**
     * Node address
     */
    address myAddr;

    //DeviceDriver driver;
    DeviceDriver* myDriver;

    /**
     * Parent address
     */
    address parentAddr;

    /**
     * Current State
     */
    char state;

    /**
     * Check if the parent is alive by sending a message to the parent and receiving an ACK.
     * Returns true if the parent replied before the timeout (alive).
     */
    bool checkParentAlive();

};

#endif