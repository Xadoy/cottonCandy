
#ifndef HEADER_MESSAGE_PROCESSOR
#define HEADER_MESSAGE_PROCESSOR

#define MESSAGE_JOIN              1
#define MESSAGE_JOIN_ACK          2
#define MESSAGE_JOIN_CFM          3
#define MESSAGE_CHECK_ALIVE       4
#define MESSAGE_REPLY_ALIVE       5
#define MESSAGE_GATEWAY_REQ       6
#define MESSAGE_NODE_REPLY        7

#define MSG_LEN_GENERIC           3
#define MSG_LEN_JOIN              3
#define MSG_LEN_JOIN_ACK          7
#define MSG_LEN_JOIN_CFM          4
#define MSG_LEN_CHECK_ALIVE       4
#define MSG_LEN_REPLY_ALIVE       3
#define MSG_LEN_GATEWAY_REQ       4


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
    uint8_t rssi;

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
    int send(DeviceDriver* driver, address destAddr);
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
    char* data; // maximum length 128 bytes

    NodeReply(address srcAddr, unsigned char numOfNodes, unsigned char seqNum, 
                unsigned char dataLength, char* data);
    int send(DeviceDriver* driver, address destAddr);
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
char* readMsgFromBuff(DeviceDriver* driver, uint8_t msgLen);

/* 
 * Helper: convert int to byte array 
 * Assume Little Endian
 */
void intToBytes(char* bytes, int intVal);

/* 
 * Helper: convert byte array to int
 * Assume Little Endian
 */
int bytesToInt(char* bytes);

#endif
