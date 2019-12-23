
#ifndef HEADER_MESSAGE_PROCESSOR
#define HEADER_MESSAGE_PROCESSOR

#define MESSAGE_JOIN              1
#define MESSAGE_JOIN_ACK          2
#define MESSAGE_JOIN_CFM          3
#define MESSAGE_CHECK_ALIVE       4
#define MESSAGE_REPLY_ALIVE       5
#define MESSAGE_GATEWAY_REQ       6
#define MESSAGE_NODE_REPLY        7

#include "DeviceDriver.h"

/* We will use polymorphism here */

class GenericMessage
{

public:
    unsigned char type;
    address srcAddr;
    
    /**
     * For every message receveid, there will be an RSSI value associated
     */
    unsigned char rssi;

    GenericMessage(unsigned char type, address srcAddr);
    // return number of bytes sent
    virtual int send(DeviceDriver* driver, address destAddr);
    void copyTypeAndAddr(char* msg);
};

/*--------------------Join Beacon-------------------*/
class Join: public GenericMessage
{
public:
    Join(address srcAddr);
};

/*--------------------JoinACK Message-------------------*/
class JoinAck: public GenericMessage
{
public:
    int hopsToGateway;
    JoinAck(address srcAddr, int hopsToGateway);

};

/*--------------------JoinCFM Message-------------------*/
class JoinCFM: public GenericMessage
{
public:
    unsigned char depth;

    JoinCFM(address srcAddr, unsigned char depth);
    int send(DeviceDriver* driver, address destAddr);
};

/*--------------------CheckAlive Message-------------------*/
class CheckAlive: public GenericMessage
{
public:
    unsigned char depth;

    CheckAlive(address srcAddr, unsigned char depth);
    int send(DeviceDriver* driver, address destAddr);
};

/*--------------------ReplyAlive Message-------------------*/
class ReplyAlive: public GenericMessage
{
public:
    ReplyAlive(address srcAddr);
};


/*--------------------GatewayRequest Message-------------------*/
class GatewayRequest: public GenericMessage
{
public:
    unsigned char seqNum;

    GatewayRequest(address srcAddr, unsigned char seqNum);
    int send(DeviceDriver* driver, address destAddr);
};

/*--------------------NodeReply Message-------------------*/
class NodeReply: public GenericMessage
{
public:
    unsigned char numOfNodes;
    unsigned char seqNum;
    unsigned char dataLength;
    char data[128];

    NodeReply(address srcAddr, unsigned char numOfNodes, unsigned char seqNum, 
                unsigned char dataLength, char data[8]);
    int send(DeviceDriver* driver, address destAddr);
};

GenericMessage* receiveMessage();

#endif
