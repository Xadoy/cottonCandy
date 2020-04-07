#ifndef HEADER_LORA_MESH
#define HEADER_LORA_MESH

#include "ForwardEngine.h"

class LoRaMesh{

public:
    /**
     * Destructor
     */
    ~LoRaMesh();

    /**
     * Constructor. Requires driver and assigned addr
     */
    LoRaMesh(byte* addr, DeviceDriver* driver);

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

    void setGatewayReqTime(unsigned long gatewayReqTime);

    unsigned long getGatewayReqTime();

    void onReceiveRequest(void(*callback)(byte**, byte*));
    void onReceiveResponse(void(*callback)(byte*, byte));


private:

  ForwardEngine* myEngine;

};

#endif
