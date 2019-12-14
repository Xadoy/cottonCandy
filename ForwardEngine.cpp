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
    return this->parentAddr;
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
        return false;
    }

    GenericMessage *msg = nullptr;
    address bestParentAddr = myAddr;

    //Send out the beacon once
    sendBeacon();

    //Give some time for the transimission and replying
    sleepForMillis(500);

    unsigned long previousTime = getTimeMillis();

    /** 
     * In this loop, for a period of DISCOVERY_TIME, the node will wait for the following types 
     * of incoming messages:
     *          1. Beacon ACK (sent by a potential parent)
     *          2. Beacon (sent by a potential child)
     *          3. Beacon CFM (sent by a child)
     * 
     * It is possible that the node did not receive any above messages at all. In this case, the
     * loop will timeout after a period of DISCOVERY_TIME. 
     */
    while (getTimeMillis() - previousTime < DISCOVERY_TIME)
    {

        //Now try to receive the message
        //TODO: receiveMessage should have a timeout value
        msg = receiveMessage();

        if(msg == nullptr){
            continue;
        }

        switch (msg->type)
        {
        case 2:
            //If it receives an ACK sent by a potential parent, compare with the current parent
            break;

        case 1:
            //If it receives an Beacon from another node (Potentail child), check the capability
            //and decide the action (Send back ACK or ignore)
            break;
        }
    }

    if (bestParentAddr != this->myAddr)
    {
        //If a parent other than itself is found. Set the parentAddr and send an CFM back
        this->parentAddr = bestParentAddr;
        sendBeaconCFM(this->parentAddr);
    }
}

bool ForwardEngine::run()
{

    while(state == INIT){
        if(join()){
            state = JOINED;
        }
    }
    
    GenericMessage *msg = nullptr;

    while (state == JOINED)
    {
        msg = receiveMessage();

        //Based on the receved message, do the corresponding actions
        switch (msg->type)
        {
        case 1:
            break;

        case 2:
            break;
        }

        //If the parent is not found anymore, exit the connection
        if (!checkParentAlive())
        {
            state = INIT;
            return 1;
        }
    }
    return 1;
}

bool ForwardEngine::checkParentAlive()
{
    sendBeacon();

    GenericMessage* msg = receiveMessage();

    if(msg!=nullptr && msg->type == 3){
        //Parent reply has been received
        return true;
    }

    return false;
}