
#ifndef HEADER_MESSAGE_PROCESSOR
#define HEADER_MESSAGE_PROCESSOR

#define MESSAGE_JOIN              1
#define MESSAGE_JOIN_ACK          2
#define MESSAGE_JOIN_CONFIRM      3
#define MESSAGE_CHECK_ALIVE       4
#define MESSAGE_REPLY_ALIVE       5
#define MESSAGE_GATEWAY_REQUEST   6
#define MESSAGE_NODE_REPLY        7

#include "DeviceDriver.h"

/* We will use polymorphism here */
/*
  Generic Message can be directly used for the following types:
  MESSAGE_JOIN
  MESSAGE_JOIN_ACK
  MESSAGE_REPLY_ALIVE
*/
class GenericMessage
{

public:
    unsigned char type;
    address srcAddr;

    GenericMessage(unsigned char type, address srcAddr);
    // return number of bytes sent
    virtual int send(DeviceDriver* driver, address destAddr);
    void copyTypeAndAddr(char* msg);
};

class Join: public GenericMessage{
};
class JoinAck: public GenericMessage{
};

class CheckAlive: public GenericMessage
{
  public:
    unsigned char depth;

    CheckAlive(unsigned char type, address srcAddr, unsigned char depth);
    int send(DeviceDriver* driver, address destAddr);
};

class ReplyAlive: public GenericMessage{
};

class JoinConfirm: public GenericMessage
{
public:
    unsigned char depth;

    JoinConfirm(unsigned char type, address srcAddr, unsigned char depth);
    int send(DeviceDriver* driver, address destAddr);
};

class GatewayRequest: public GenericMessage
{
public:
    unsigned char seqNum;

    GatewayRequest(unsigned char type, address srcAddr, unsigned char seqNum);
    int send(DeviceDriver* driver, address destAddr);
};

class NodeReply: public GenericMessage
{
public:
    unsigned char numOfNodes;
    unsigned char seqNum;
    unsigned char dataLength;
    char data[8];

    NodeReply(unsigned char type, address srcAddr, unsigned char numOfNodes, unsigned char seqNum, 
                unsigned char dataLength, char data[8]);
    int send(DeviceDriver* driver, address destAddr);
};

GenericMessage* receiveMessage();

#endif
