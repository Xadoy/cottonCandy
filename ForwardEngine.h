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

/* Setting the first bit of the address to 1 indicates a gateways */
#define GATEWAY_ADDRESS_MASK 0x80

/* The default time for discovery is 10 seconds */
#define DISCOVERY_TIMEOUT 10000

/* The default timeout value for receiving a message is 5 seconds */
#define RECEIVE_TIMEOUT 5000

/* The RSSI threshold for choosing a parent node */
#define RSSI_THRESHOLD 60

/* The maximum number of children a node can have */
#define MAX_NUM_CHILDREN 5

/* The default time interval for checking if the parent is alive */
#define DEFAULT_CHECK_ALIVE_INTERVAL 30000

/* The default time interval for checking if the parent is alive */
#define CHECK_ALIVE_TIMEOUT 10000

struct ParentInfo{
    unsigned long lastAliveTime;
    byte hopsToGateway;
    
    byte parentAddr[2];
    uint8_t Rssi;  
};

struct ChildNode{
    byte* nodeAddr;

    ChildNode* next;
};

class ForwardEngine{

public:
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
     * TODO: Add the function pointer for callback
     */
    ForwardEngine(byte* addr, DeviceDriver* driver);

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


    //Setter for the node address
    void setAddr(byte* addr);

    /**
     * Getter for the node address
     */
    byte* getMyAddr();

    /**
     * Getter for the parent address
     */
    byte* getParentAddr();

private:
    /**
     * Node address
     */
    byte* myAddr;

    /**
     * DeviceDriver driver;
     */ 
    DeviceDriver* myDriver;

    /**
     * A record of current parent
     */
    ParentInfo myParent;

    /**
     * Hops to the gateway
     */ 
    byte hopsToGateway; 

    /**
     * Current State
     */
    char state;

    /**
     * Number of direct children currently connected to
     */ 
    uint8_t numChildren;

    /**
     * A linked list of children nodes
     */
    ChildNode* childrenList;

    unsigned long checkAliveInterval = 10000;

};

#endif
