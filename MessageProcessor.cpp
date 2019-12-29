#include "MessageProcessor.h"
#include <string.h>
#include "Utilities.h"


GenericMessage::GenericMessage(unsigned char type, byte* srcAddr)
{
    this->type = type;
    this->srcAddr = new byte[2];    
    
    memcpy(this->srcAddr, srcAddr, 2);
}

//Note:Little Endian reverse the order of bytes

void GenericMessage::copyTypeAndAddr(byte* msg)
{
    msg[0] = this->type;
    //reverse the bytes
    msg[1] = this->srcAddr[0];
    msg[2] = this->srcAddr[1];
    //msg[1] = ((unsigned char *)&(this->srcAddr))[1];
    //msg[2] = ((unsigned char *)&(this->srcAddr))[0];
}

int GenericMessage::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    byte msg[MSG_LEN_GENERIC]; 
    copyTypeAndAddr(msg);

    return ( driver->send(destAddr, msg, MSG_LEN_GENERIC) );
}

GenericMessage::~GenericMessage(){
    free(srcAddr);
}

/*--------------------Join Beacon-------------------*/
Join::Join(byte* srcAddr) : GenericMessage(MESSAGE_JOIN, srcAddr)
{
}


/*--------------------JoinACK Message-------------------*/
JoinAck::JoinAck(byte* srcAddr, int hopsToGateway) : GenericMessage(MESSAGE_JOIN_ACK, srcAddr)
{
    this->hopsToGateway = hopsToGateway;
}

JoinAck::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    char msg[MSG_LEN_JOIN_ACK];
    copyTypeAndAddr(msg);
    // convert int to byte array
    // when JoinAck is sent, it guarantees the hopsToGateway is positive
    intToBytes(msg+3, this->hopsToGateway);

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------JoinCFM Message-------------------*/
JoinCFM::JoinCFM(byte* srcAddr, unsigned char depth) : GenericMessage(MESSAGE_JOIN_CFM, srcAddr)
{
    this->depth = depth;
}

int JoinCFM::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    char msg[MSG_LEN_JOIN_CFM];
    copyTypeAndAddr(msg);
    msg[3] = this->depth;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------CheckAlive Message-------------------*/
CheckAlive::CheckAlive(byte* srcAddr, unsigned char depth) : GenericMessage(MESSAGE_CHECK_ALIVE, srcAddr)
{
    this->depth = depth;
}

int CheckAlive::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    char msg[MSG_LEN_CHECK_ALIVE];
    copyTypeAndAddr(msg);
    msg[3] = this->depth;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}


/*--------------------ReplyAlive Message-------------------*/
ReplyAlive::ReplyAlive(byte* srcAddr) : GenericMessage(MESSAGE_REPLY_ALIVE, srcAddr){
  
}

/*--------------------GatewayRequest Message-------------------*/
GatewayRequest::GatewayRequest(byte* srcAddr, unsigned char seqNum): GenericMessage(MESSAGE_GATEWAY_REQ, srcAddr)
{
    this->seqNum = seqNum;
}

int GatewayRequest::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    char msg[MSG_LEN_GATEWAY_REQ];
    copyTypeAndAddr(msg);
    msg[3] = this->seqNum;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------NodeReply Message-------------------*/
NodeReply::NodeReply(byte* srcAddr, unsigned char numOfNodes, unsigned char seqNum, 
                unsigned char dataLength, byte* data) : GenericMessage(MESSAGE_NODE_REPLY, srcAddr)
{
    this->type = type;
    this->srcAddr = srcAddr;
    this->numOfNodes = numOfNodes;
    this->seqNum = seqNum;
    this->dataLength = dataLength;
    memcpy(this->data, data, dataLength);
}

int NodeReply::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    char msg[this->dataLength + 6];
    copyTypeAndAddr(msg);

    msg[3] = this->numOfNodes;
    msg[4] = this->seqNum;
    msg[5] = this->dataLength;
    memmove(msg + 6, this->data, this->dataLength);

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

GenericMessage* receiveMessage(DeviceDriver* driver, unsigned long timeout)
{
    unsigned long startTime = getTimeMillis();
    char msgType = -1;
    GenericMessage* msg = nullptr;

    while(getTimeMillis() - startTime < timeout)
    {
        // get first char, check msg type
        msgType = driver->recv();
        if(msgType <= 0)
            continue;

        // get the whole message from device buffer
        switch(msgType)
        {
        case MESSAGE_JOIN:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_JOIN - 1);

            // get what we need for Join
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);

            msg = new Join(srcAddr);
            delete[] buff;
            break;
        }        

        case MESSAGE_JOIN_ACK:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_JOIN_ACK - 1);

            // get what we need for JoinAck
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);

            int hopsToGateway = bytesToInt(buff+2);

            msg = new JoinAck(srcAddr, hopsToGateway);
            delete[] buff;
            break;
        }

        case MESSAGE_JOIN_CFM:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_JOIN_CFM - 1);

            // get what we need for JoinCFM
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);

            unsigned char depth = buff[2];

            msg = new JoinCFM(srcAddr, depth);
            delete[] buff;
            break;
        }

        case MESSAGE_CHECK_ALIVE:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_CHECK_ALIVE - 1);

            // get what we need for CheckAlive
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);

            unsigned char depth = buff[2];

            msg = new CheckAlive(srcAddr, depth);
            delete[] buff;
            break;
        }

        case MESSAGE_REPLY_ALIVE:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_REPLY_ALIVE - 1);

            // get what we need for ReplyAlive
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);

            msg = new ReplyAlive(srcAddr);
            delete[] buff;
            break;
        }

        case MESSAGE_GATEWAY_REQ:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_GATEWAY_REQ - 1);

            // get what we need for GatewayRequest
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);

            unsigned char depth = buff[2];

            msg = new GatewayRequest(srcAddr, depth);
            delete[] buff;
            break;
        }

        case MESSAGE_NODE_REPLY:
        {
            // we have already read the msg type
            // need to know the data length before getting the data
            byte srcAddr[2];
            srcAddr[0] = driver->recv();
            srcAddr[1] = driver->recv();

            unsigned char numOfNodes = driver->recv();
            unsigned char seqNum = driver->recv();
            unsigned char dataLength = driver->recv();

            byte* data = readMsgFromBuff(driver, dataLength);

            msg = new NodeReply(srcAddr, numOfNodes, seqNum, dataLength, data);
            delete[] data;
            break;
        }
        
        default:
            return nullptr;
        }
        msg->rssi = driver->getLastMessageRssi();
        return msg;
    }

   return nullptr;
}


/*-------------------- Helpers -------------------*/
byte* readMsgFromBuff(DeviceDriver* driver, uint8_t msgLen)
{
    byte* buff = new byte[msgLen];
    /*
    for(int i = 0; i < msgLen; i++)
    {
        buff[i] = driver->recv();
    }
    */
    int i = 0;
    
    while(i < msgLen){
        byte c = driver->recv();
        if(c == 0xFF){
            continue;
        }

        buff[i] = c;
        Serial.print(buff[i], HEX);
        Serial.print(" ");
        i++;
    }
    Serial.print("\n");
    return buff;
}

void intToBytes(byte* bytes, int intVal)
{
    bytes[3] = (intVal >> 24) & 0xFF;
    bytes[2] = (intVal >> 16) & 0xFF;
    bytes[1] = (intVal >> 8) & 0xFF;
    bytes[0] = intVal & 0xFF;
}

int bytesToInt(byte* bytes)
{
    int ret;
    memcpy(&ret, bytes, 4);
    return ret;
}

