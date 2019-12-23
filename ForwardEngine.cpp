#include "ForwardEngine.h"

//Default Constructor
ForwardEngine::ForwardEngine()
{

    myAddr = 0;
}

ForwardEngine::ForwardEngine(address addr, DeviceDriver* driver)
{
    myAddr = addr;
    myDriver = driver;

    //Initialize the parent. Set the parent to itself.
    myParent.parentAddr = myAddr;
    myParent.hopsToGateway = -1;
    myParent.lastRssi = -100;

    numChildren = 0;
}

ForwardEngine::~ForwardEngine()
{
    //clean up
}

void ForwardEngine::setAddr(address addr)
{
    this->myAddr = addr;
}

address ForwardEngine::getMyAddr()
{
    return this->myAddr;
}

address ForwardEngine::getParentAddr()
{
    return this->myParent.parentAddr;
}

/**
 * The join function is responsible for sending out a beacon to discover neighboring 
 * nodes. After sending out the beacon, the node will receive messages for a given
 * period of time. Since the node might receive multiple replies of its beacon, as well
 * as the beacons from other nearby nodes, it waits for a period of time to collect info
 * from the nearby neighbors, and pick the best parent using the replies received.
 * 
 * Returns: The received message during the discovery
 */
bool ForwardEngine::join()
{
    if (state != INIT)
    {
        //The node has already joined a network
        return false;
    }

    GenericMessage *msg = nullptr;
    
    ParentInfo bestParentCandidate = myParent;

    Join beacon(myAddr);
    
    //Send out the beacon once to discover nearby nodes
    beacon.send(myDriver, BROADCAST_ADDR);

    //Give some time for the transimission and replying
    sleepForMillis(500);

    unsigned long previousTime = getTimeMillis();

    /** 
     * In this loop, for a period of DISCOVERY_TIME, the node will wait for the following types 
     * of incoming messages:
     *          1. Beacon ACK (sent by a potential parent)
     *          4. For any other types of message, the node will discard them
     * 
     * It is possible that the node did not receive any above messages at all. In this case, the
     * loop will timeout after a period of DISCOVERY_TIMEOUT. 
     */
    while (getTimeMillis() - previousTime < DISCOVERY_TIMEOUT)
    {

        //Now try to receive the message
        //TODO: receiveMessage should have a timeout value
        msg = receiveMessage();

        if(msg == nullptr){
            //If no message has been received
            continue;
        }

        address nodeAddr = msg->srcAddr;

        switch (msg->type)
        {
        case MESSAGE_JOIN_ACK:
            //If it receives an ACK sent by a potential parent, compare with the current parent candidate
            int newHopsToGateway = ((JoinAck*)msg) -> hopsToGateway;

            //Need to find another way to get the RSSI value
            uint8_t newRssi = myDriver->getLastMessageRssi();

            if(newHopsToGateway != -1){
                //The remote node has a connection to the gateway
                if (bestParentCandidate.hopsToGateway != -1){
                    //Case 1: Both the current parent candidate and new node are connected to the gateway
                    //Choose the candidate with the minimum hops to the gateway while the RSSI is over the threshold
                    //Note that the RSSI value returned here is positive, thus the smaller RSSI is better.

                    if(newRssi <= RSSI_THRESHOLD && newHopsToGateway < bestParentCandidate.hopsToGateway){
                        bestParentCandidate.parentAddr = nodeAddr;
                        bestParentCandidate.hopsToGateway = newHopsToGateway;
                        bestParentCandidate.lastRssi = newRssi;
                    }
                    
                }else{
                    //Case 2: Only the new node is connected to the gateway
                    //We always favor the candidate with a connection to the gateway
                    bestParentCandidate.parentAddr = nodeAddr;
                    bestParentCandidate.hopsToGateway = newHopsToGateway;
                    bestParentCandidate.lastRssi = newRssi;
                }

            }
            //This case is currently ignored
            /*
            else if (bestParentCandidate.hopsToGateway == -1){
                //Case 3: Both the current and new parent candidates does not have a connection to the gateway
                //Compare the node address. The smaller node address should be the parent (gateway address is always larger than regular node address)
                if(nodeAddr < bestParentCandidate.parentAddr){
                    bestParentCandidate.parentAddr = nodeAddr;
                    bestParentCandidate.hopsToGateway = newHopsToGateway;
                    bestParentCandidate.lastRssi = newRssi;
                }
            }
            */

            //Other cases involve: new node -> not connected to gateway, current best parent -> connected to the gateway
            //In this case we will not update the best parent candidate
            break;
        }
    }

    if(bestParentCandidate.parentAddr != myAddr){
        myParent = bestParentCandidate;
        JoinCFM cfm(myAddr, numChildren);
        cfm.send(myDriver, myParent.parentAddr);
        return true;
    }else{
        return false;
    }
}

bool ForwardEngine::run()
{

    //If the node is a gateway, it does not have to join the network to operate
    //Gateway is distinguished by the highest bit = 1
    if(myAddr & 0x80){
        state = JOINED;
    }

    //If it is a regular node, it needs to join the network to operate
    while(state == INIT){
        if(join()){
            state = JOINED;
        }else{
            Serial.println("Joining unsuccessful. Retry joining in 5 seconds");
            sleepForMillis(5000);
        }
    }
    
    GenericMessage *msg = nullptr;

    while (state == JOINED)
    {
        msg = receiveMessage();

        if(msg != nullptr){

            address nodeAddr = msg->srcAddr;

            //Based on the receved message, do the corresponding actions
            switch (msg->type)
            {
            case MESSAGE_JOIN:
                //TODO: Send back JOIN_ACK with information
                JoinAck ack(myAddr, myParent.hopsToGateway + 1);
                ack.send(myDriver, nodeAddr);
                break;

            case MESSAGE_JOIN_CFM:
                //TODO: Add the node to the children list
                break;
            }

        }

        //Check if the parent is still alive
        //If the parent is not found anymore, exit the connection
        if (!checkParentAlive())
        {
            state = INIT;
            break;
        }
    }

    return 1;
}

bool ForwardEngine::checkParentAlive()
{
    //TODO: Send out the checkAlive message
    CheckAlive check(myAddr, 0);
    check.send(myDriver, myParent.parentAddr);

    GenericMessage* msg = receiveMessage();

    if(msg!=nullptr && msg->type == MESSAGE_REPLY_ALIVE){
        //Parent reply has been received
        return true;
    }

    return false;
}