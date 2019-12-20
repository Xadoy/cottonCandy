
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
    address addr;

    virtual void send();
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

    void send();
};

class ReplyAlive: public GenericMessage{
};

class JoinConfirm: public GenericMessage
{
public:
    unsigned char depth;

    void send();
};

class GatewayRequest: public GenericMessage
{
public:
    unsigned char seqNum;

    void send();
};

class NodeReply: public GenericMessage
{
public:
    unsigned char numOfNodes;
    unsigned char seqNum;
    unsigned char dataLength;
    char data[8];

    void send();
};

GenericMessage* receiveMessage();

#endif
