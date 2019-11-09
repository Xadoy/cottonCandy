#include "ForwardEngine.h"

//Default Constructor
ForwardEngine::ForwardEngine(){
    
    myAddr = 0;
}

ForwardEngine::~ForwardEngine(){
    //clean up
}

/**
 * In the discovery process, a node will send out beacons periodically to discover 
 * neighboring nodes. The overall discovery process takes ~30 seconds to allow the
 * node to pick the best neighbor node to connect to.
 */
GenericMessage* ForwardEngine::discovery(){
    if(state != INIT){
        return nullptr;
    }

    GenericMessage* msg = nullptr;
    address bestParentAddr = myAddr;

    unsigned long startTime = getTimeMillis();

    while(getTimeMillis() - startTime <= 30000){
        
        sendBeacon();
        msg = receiveMessage();
        
        //If it receives an ACK sent by a potential parent, compare with the current parent
        if(msg != nullptr && msg-> type == 2){
            state = SELECT;

            
        }

        //If it receives an Beacon from another node (Potentail child), check the capability
        //and decide the action (Sendback ACK or ignore)
        if(msg != nullptr && msg->type == 1){

        }
    }


}

int ForwardEngine::join(){
    return 1;
}