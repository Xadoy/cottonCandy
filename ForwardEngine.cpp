#include "ForwardEngine.h"
#include <string.h>

ForwardEngine::ForwardEngine(byte *addr, DeviceDriver *driver)
{
    myAddr[0] = addr[0];
    myAddr[1] = addr[1];

    myDriver = driver;

    //A node is its own parent initially
    memcpy(myParent.parentAddr, myAddr, 2);
    myParent.hopsToGateway = 255;

    numChildren = 0;
    //childrenList = nullptr;

    //Here we will set the random seed to the node its own address
    //analogRead(A0) can also be used. Interesting to find out if it is better
    unsigned long seed = myAddr[0] << 8 + myAddr[1];

    randomSeed(seed);
}

ForwardEngine::~ForwardEngine()
{
    //TODO: need to do some clean up here
    /*
    ChildNode *iter = childrenList;
    while (iter != nullptr)
    {
        ChildNode *temp = iter;

        iter = temp->next;
        delete temp;
    }
    */
}

void ForwardEngine::setAddr(byte *addr)
{
    memcpy(myAddr, addr, 2);
}

byte *ForwardEngine::getMyAddr()
{
    return this->myAddr;
}

byte *ForwardEngine::getParentAddr()
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
    Join beacon(myAddr, BROADCAST_ADDR);

    //Send out the beacon once to discover nearby nodes
    beacon.send(myDriver, BROADCAST_ADDR);

    //Give some time for the transimission and replying
    //sleepForMillis(500);

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
    while ((unsigned long)(getTimeMillis() - previousTime) < DISCOVERY_TIMEOUT)
    {

        //Now try to receive the message
        //TODO: receiveMessage should have a timeout value
        msg = receiveMessage(myDriver, RECEIVE_TIMEOUT);

        if (msg == nullptr)
        {
            //If no message has been received
            continue;
        }

        byte *nodeAddr = msg->srcAddr;
        Serial.print("Received msg type = ");
        Serial.println(msg->type);
        switch (msg->type)
        {
        case MESSAGE_JOIN_ACK:
        {
            Serial.print(F("MESSAGE_JOIN_ACK: src=0x"));
            Serial.print(nodeAddr[0], HEX);
            Serial.print(nodeAddr[1], HEX);
            Serial.print(" rssi=");
            Serial.println(msg->rssi, DEC);

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

                    if (msg->rssi >= RSSI_THRESHOLD && newHopsToGateway < bestParentCandidate.hopsToGateway)
                    {
                        memcpy(bestParentCandidate.parentAddr, nodeAddr, 2);
                        bestParentCandidate.hopsToGateway = newHopsToGateway;
                        bestParentCandidate.Rssi = msg->rssi;

                        Serial.println(F("This is a better parent (closer to gateway)"));
                    }
                }
                else
                {
                    //Case 2: Only the new node is connected to the gateway
                    //We always favor the candidate with a connection to the gateway
                    memcpy(bestParentCandidate.parentAddr, nodeAddr, 2);
                    bestParentCandidate.hopsToGateway = newHopsToGateway;
                    bestParentCandidate.Rssi = msg->rssi;
                    Serial.println(F("This is the first new parent"));
                }
            }else{
                Serial.println(F("The node does not have a path to gateway. Discard"));
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

        delete msg;
    }

    Serial.println("Discovery timeout");

    if (bestParentCandidate.parentAddr[0] != myAddr[0] || bestParentCandidate.parentAddr[1] != myAddr[1])
    {
        //New parent has found
        Serial.print(F("bestParentCandidate.parentAddr = 0x"));
        Serial.print(bestParentCandidate.parentAddr[0], HEX);
        Serial.println(bestParentCandidate.parentAddr[1], HEX);

        myParent = bestParentCandidate;

        hopsToGateway = bestParentCandidate.hopsToGateway + 0b1;

        Serial.print(F(" HopsToGateway = "));
        Serial.println(hopsToGateway);

        Serial.println(F("Send JoinCFM to parent"));
        //Send a confirmation to the parent node
        JoinCFM cfm(myAddr, myParent.parentAddr, numChildren);

        cfm.send(myDriver, myParent.parentAddr);

        //Assign the alive timestamp to the parent
        myParent.lastAliveTime = getTimeMillis();
        
        
        myParent.requireChecking = false;

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

    Serial.println(F("Joining successful"));

    //bool checkingParent = false;
    unsigned long checkingStartTime = 0;

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

                JoinAck ack(myAddr, nodeAddr, hopsToGateway);

                // Introduce some random time backoff to prevent collision
                // From our experiments, we noticed packet losses when multiple nodes send joinACK instantly
                // upon receiving a join message. This cause some packets to go missing (Even LBT in EBYTE can
                // not help since the sending happened at almost the same time)

                long backoff = random(MIN_BACKOFF_TIME, MAX_BACKOFF_TIME);

                Serial.print(F("Sleep for some time before sending JoinAck: "));
                Serial.println(backoff);

                sleepForMillis(backoff);

                ack.send(myDriver, nodeAddr);

                Serial.print(F("MESSAGE_JOIN: src=0x"));
                Serial.print(nodeAddr[0], HEX);
                Serial.println(nodeAddr[1], HEX);

                break;
            }
            case MESSAGE_JOIN_CFM:
            {
                //TODO: The current link list might not be necessary. Need to implement this
                //Add the new child to the linked list (Insert in the beginning of the linked list)
                /*
                ChildNode *node = new ChildNode();
                node->nodeAddr[0] = msg->srcAddr[0];
                node->nodeAddr[1] = msg->srcAddr[1];

                node->next = childrenList;

                childrenList = node;
                */
                numChildren++;

                Serial.print("\nA new child has joined: 0x");
                Serial.print(nodeAddr[0], HEX);
                Serial.println(nodeAddr[1], HEX);
                break;
            }
            case MESSAGE_REPLY_ALIVE:
            {
                //We do not need to check the message src address here since the parent should
                //only unicast the reply message (Driver does the filtering).
                Serial.println(F("ReplyAlive from parent"));
                //If we have previously issued a checkAlive message
                if (myParent.requireChecking)
                {
                    //The parent node is proven to be alive
                    myParent.requireChecking = false;

                    //Update the last time we confirmed when the parent node was alive
                    myParent.lastAliveTime = getTimeMillis();
                }
                break;
            }
            case MESSAGE_CHECK_ALIVE:
            {
                //Parent replies back to the child node
                Serial.println("I got checked by my son");
                ReplyAlive reply(myAddr, nodeAddr);
                reply.send(myDriver, nodeAddr);
                break;
            }
            }

            delete msg;
        }
        else
        {
            Serial.println(F("No message has been received"));
        }
        Serial.print(F("Free Memory= "));
        Serial.println(freeMemory());
        //The gateway does not need to check its parent
        if (myAddr[0] & GATEWAY_ADDRESS_MASK)
        {
            continue;
        }

        unsigned long currentTime = getTimeMillis();

        //The parent is currently being checked (CheckAlive Message has been sent already), but the reply has not been received yet
        if (myParent.requireChecking)
        {
            //If the reply has not been received in 10 seonds
            if (currentTime - checkingStartTime >= CHECK_ALIVE_TIMEOUT)
            {
                state = INIT;
                Serial.println(F("CheckAlive Timeout"));
                //loop will break
            }
        }
        //If the parent is not being checked and has not been checked in the past 30 seconds, we might need to check the parent liveness
        else if ((unsigned long)(currentTime - myParent.lastAliveTime) >= checkAliveInterval)
        {
            Serial.println(F("Time to check parent"));
            myParent.requireChecking = true;
            //Send out the checkAlive message to the parent
            CheckAlive checkMsg(myAddr, myParent.parentAddr, 0);
            checkMsg.send(myDriver, myParent.parentAddr);

            //record the current time
            checkingStartTime = getTimeMillis();
            
            Serial.print(F("Checking start at "));
            Serial.println(checkingStartTime);
        }
    }

    //We have disconnected from the parent
    myParent.parentAddr[0] = myAddr[0];
    myParent.parentAddr[1] = myAddr[1];
    //myParent.lastAliveTime = getTimeMillis();

    return 1;
}
