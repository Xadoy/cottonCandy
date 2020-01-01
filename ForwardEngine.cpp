#include "ForwardEngine.h"
#include <string.h>

ForwardEngine::ForwardEngine(byte* addr, DeviceDriver *driver)
{
    myAddr = new byte[2];
    
    myAddr[0] = addr[0];
    myAddr[1] = addr[1];

    myDriver = driver;

    //Initialize the parent to NULL
    memcpy(myParent.parentAddr, myAddr, 2);
    myParent.hopsToGateway = 255;

    numChildren = 0;
    childrenList = nullptr;
}

ForwardEngine::~ForwardEngine()
{
    ChildNode *iter = childrenList;
    while (iter != nullptr)
    {
        ChildNode *temp = iter;

        iter = temp->next;
        delete temp;
    }
}

void ForwardEngine::setAddr(byte* addr)
{
    memcpy(myAddr, addr, 2);
}

byte* ForwardEngine::getMyAddr()
{
    return this->myAddr;
}

byte* ForwardEngine::getParentAddr()
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
 * Returns: True if the node has joined a parent
 */
bool ForwardEngine::join()
{
    if (state != INIT)
    {
        //The node has already joined a network
        return true;
    }

    GenericMessage *msg = nullptr;

    ParentInfo bestParentCandidate = myParent;

    //Serial.print("myAddr = 0x");
    //Serial.print(myAddr[0], HEX);
    //Serial.println(myAddr[1], HEX);
    Join beacon(myAddr);

    //Send out the beacon once to discover nearby nodes
    beacon.send(myDriver, BROADCAST_ADDR);

    //Give some time for the transimission and replying
    sleepForMillis(500);

    //Serial.print("Wait for reply: timeout = ");
    //Serial.println(DISCOVERY_TIMEOUT);

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
        msg = receiveMessage(myDriver, RECEIVE_TIMEOUT);

        if (msg == nullptr)
        {
            //If no message has been received
            continue;
        }

        byte* nodeAddr = msg->srcAddr;
        Serial.print("Received msg type = ");
        Serial.print(msg->type);
        switch (msg->type)
        {
            case MESSAGE_JOIN_ACK:
            {
                //Serial.print("MESSAGE_JOIN_ACK: src=0x");
                //Serial.print(nodeAddr, HEX);
                //Serial.print(" rssi=");
                //Serial.println(msg->rssi, HEX);

                //If it receives an ACK sent by a potential parent, compare with the current parent candidate
                byte newHopsToGateway = ((JoinAck *)msg)->hopsToGateway;

                if (newHopsToGateway != 255)
                {
                    //The remote node has a connection to the gateway
                    if (bestParentCandidate.hopsToGateway != 255)
                    {
                        //Case 1: Both the current parent candidate and new node are connected to the gateway
                        //Choose the candidate with the minimum hops to the gateway while the RSSI is over the threshold
                        //Note that the RSSI value returned here is positive, thus the smaller RSSI is better.

                        if (msg->rssi <= RSSI_THRESHOLD && newHopsToGateway < bestParentCandidate.hopsToGateway)
                        {
                            memcpy(bestParentCandidate.parentAddr, nodeAddr, 2);
                            bestParentCandidate.hopsToGateway = newHopsToGateway;
                            bestParentCandidate.Rssi = msg->rssi;
                            bestParentCandidate.lastAliveTime = getTimeMillis();
                        }
                    }
                    else
                    {
                        //Case 2: Only the new node is connected to the gateway
                        //We always favor the candidate with a connection to the gateway
                        memcpy(bestParentCandidate.parentAddr, nodeAddr, 2);
                        bestParentCandidate.hopsToGateway = newHopsToGateway;
                        bestParentCandidate.Rssi = msg->rssi;
                        bestParentCandidate.lastAliveTime = getTimeMillis();
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
                        bestParentCandidate.Rssi = newRssi;
                    }
                }
                */

                //Other cases involve: new node -> not connected to gateway, current best parent -> connected to the gateway
                //In this case we will not update the best parent candidate
                break;
            }
            default:
                //Serial.print("MESSAGE: type=");
                //Serial.print(msg->type, HEX);
                //Serial.print(" src=0x");
                //Serial.println(nodeAddr, HEX);
                break;
        }

        free(msg);
    }

    Serial.println("Discovery timeout");

    if (bestParentCandidate.parentAddr[0] != myAddr[0] || bestParentCandidate.parentAddr[1] != myAddr[1] )
    {
        Serial.print("bestParentCandidate.parentAddr = 0x");
        Serial.print(bestParentCandidate.parentAddr[0], HEX);
        Serial.println(bestParentCandidate.parentAddr[1], HEX);
        myParent = bestParentCandidate;
        hopsToGateway = bestParentCandidate.hopsToGateway + 0b1;

        Serial.print(F(" HopsToGateway = "));
        Serial.println(hopsToGateway);

        Serial.println(F("Send JoinCFM to parent"));
        //Send a confirmation to the parent node
        JoinCFM cfm(myAddr, numChildren);
        
        cfm.send(myDriver, myParent.parentAddr);
        return true;
    }
    else
    {
        return false;
    }
}

bool ForwardEngine::run()
{

    //If the node is a gateway, it does not have to join the network to operate
    //Gateway is distinguished by the highest bit = 1
    if (myAddr[0] & GATEWAY_ADDRESS_MASK)
    {
        state = JOINED;

        //Gateway has the cost of 0
        hopsToGateway = 0;
    }
    else
    {
        state = INIT;

        //Uninitilized gateway cost
        hopsToGateway = 255;

        //If it is a regular node, it needs to join the network to operate
        while (state == INIT)
        {
            if (join())
            {
                state = JOINED;
            }
            else
            {
                Serial.println(F("Joining unsuccessful. Retry joining in 5 seconds"));
                sleepForMillis(5000);
            }
        }
    }

    Serial.println("Joining successful");

    bool checkingParent = false;
    unsigned long prevAliveCheckTime = getTimeMillis();

    GenericMessage *msg = nullptr;

    //The core network operations are carried out here
    while (state == JOINED)
    {
        msg = receiveMessage(myDriver, RECEIVE_TIMEOUT);

        if (msg != nullptr)
        {

            byte* nodeAddr = msg->srcAddr;

            //Based on the receved message, do the corresponding actions
            switch (msg->type)
            {
            case MESSAGE_JOIN:
            {
                //TODO: May need a limit for number of children

                JoinAck ack(myAddr, hopsToGateway);
                ack.send(myDriver, nodeAddr);

                Serial.print("MESSAGE_JOIN: src=0x");
                Serial.print(nodeAddr[0], HEX);
                Serial.println(nodeAddr[1], HEX);

                break;
            }
            case MESSAGE_JOIN_CFM:
            {
                //Add the new child to the linked list (Insert in the beginning of the linked list)

                ChildNode *node = new ChildNode();
                node->nodeAddr = msg->srcAddr;
                node->next = childrenList;

                childrenList = node;
                numChildren++;

                Serial.print("A new child has joined: 0x");
                Serial.print(nodeAddr[0], HEX);
                Serial.println(nodeAddr[1], HEX);
                break;
            }
            case MESSAGE_REPLY_ALIVE:
            {
                //We do not need to check the message src address here since the parent should
                //only unicast the reply message (Driver does the filtering).

                //If we have previously issued a checkAlive message
                if (checkingParent)
                {
                    //The parent node is proven to be alive
                    checkingParent = false;

                    //Update the last time we confirmed when the parent node was alive
                    myParent.lastAliveTime = getTimeMillis();
                }
                break;
            }
            case MESSAGE_CHECK_ALIVE:
            {
                //Parent replies back to the child node
                ReplyAlive reply(myAddr);
                reply.send(myDriver, nodeAddr);
                break;
            }
            }
        }

        if (msg != nullptr)
        {
            free(msg);
        }

        //The gateway does not need to check its parent
        if (myAddr[0] & GATEWAY_ADDRESS_MASK){
            continue;
        }

        //If we are not current waiting for MESSAGE_REPLY_ALIVE from the parent
        if (!checkingParent)
        {
            if (getTimeMillis() - myParent.lastAliveTime > checkAliveInterval)
            {
                //We should now check if the parent is alive

                //Send out the checkAlive message to the parent
                CheckAlive checkMsg(myAddr, 0);
                checkMsg.send(myDriver, myParent.parentAddr);

                checkingParent = true;
                prevAliveCheckTime = getTimeMillis();
            }
        }
        else
        {
            //If we are still waiting for MESSAGE_REPLY_ALIVE from the parent
            //We should check if the waiting is timed out, i.e. the parent will be treated as dead if it does not reply in time.

            if (getTimeMillis() - prevAliveCheckTime > CHECK_ALIVE_TIMEOUT)
            {

                //Redundant if checkingParent is a local variable
                checkingParent = false;

                state = INIT;

                return 1;
            }
        }
    }

    return 1;
}
