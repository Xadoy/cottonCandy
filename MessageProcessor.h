
#ifndef HEADER_MESSAGE_PROCESSOR
#define HEADER_MESSAGE_PROCESSOR

#define MESSAGE_JOIN              1
#define MESSAGE_JOIN_ACK          2
#define MESSAGE_JOIN_CFM          3
#define MESSAGE_CHECK_ALIVE       4
#define MESSAGE_REPLY_ALIVE       5
#define MESSAGE_GATEWAY_REQ       6
#define MESSAGE_NODE_REPLY        7

#define MSG_LEN_GENERIC           5
#define MSG_LEN_JOIN              5
#define MSG_LEN_JOIN_ACK          6
#define MSG_LEN_JOIN_CFM          6
#define MSG_LEN_CHECK_ALIVE       6
#define MSG_LEN_REPLY_ALIVE       5
#define MSG_LEN_GATEWAY_REQ       6
#define MSG_LEN_HEADER_NODE_REPLY 7


#include "DeviceDriver.h"

/* We will use polymorphism here */

class GenericMessage
{

public:
    byte type;
    byte srcAddr[2];
    byte destAddr[2];
    
    /**
     * For every message receveid, there will be an RSSI value associated
     */
    int rssi;

    GenericMessage(byte type, byte* srcAddr, byte* destAddr);
    // return number of bytes sent
    virtual int send(DeviceDriver* driver, byte* destAddr);
    void copyTypeAndAddr(byte* msg);

    ~GenericMessage();
};

/*--------------------Join Beacon-------------------*/
class Join: public GenericMessage
{
public:
    Join(byte* srcAddr, byte* destAddr);
};

/*--------------------JoinACK Message-------------------*/
class JoinAck: public GenericMessage
{
public:
    byte hopsToGateway;
    JoinAck(byte* srcAddr, byte* destAddr, byte hopsToGateway);
    int send(DeviceDriver* driver, byte* destAddr);
};

/*--------------------JoinCFM Message-------------------*/
class JoinCFM: public GenericMessage
{
public:
    byte depth;

    JoinCFM(byte* srcAddr, byte* destAddr, byte depth);
    int send(DeviceDriver* driver, byte* destAddr);
};

/*--------------------CheckAlive Message-------------------*/
class CheckAlive: public GenericMessage
{
public:
    byte depth;

    CheckAlive(byte* srcAddr, byte* destAddr, byte depth);
    int send(DeviceDriver* driver, byte* destAddr);
};

/*--------------------ReplyAlive Message-------------------*/
class ReplyAlive: public GenericMessage
{
public:
    ReplyAlive(byte* srcAddr, byte* destAddr);
};


/*--------------------GatewayRequest Message-------------------*/
class GatewayRequest: public GenericMessage
{
public:
    byte seqNum;

    GatewayRequest(byte* srcAddr, byte* destAddr, byte seqNum);
    int send(DeviceDriver* driver, byte* destAddr);
};

/*--------------------NodeReply Message-------------------*/
class NodeReply: public GenericMessage
{
public:
    byte seqNum;
    byte dataLength;
    byte* data; // maximum length 64 bytes

    NodeReply(byte* srcAddr, byte* destAddr, byte seqNum, 
                byte dataLength, byte* data);
    ~NodeReply();
    int send(DeviceDriver* driver, byte* destAddr);
};

/*
 * Read from device buffer, construct a message and return a pointer to it
 * !! Caller needs to free the memory after using the returned pointer
 */
GenericMessage* receiveMessage(DeviceDriver* driver, unsigned long timeout);


/*
 * Read certain bytes from device buffer
 * !! Caller needs to free the memory after using the returned pointer
 */
byte* readMsgFromBuff(DeviceDriver* driver, uint8_t msgLen);

#endif
