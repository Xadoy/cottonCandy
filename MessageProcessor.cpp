#include "MessageProcessor.h"
#include <string.h>
#include "Utilities.h"


GenericMessage::GenericMessage(byte type, byte* srcAddr)
{
    this->type = type;
    this->srcAddr = new byte[2];    
    
    memcpy(this->srcAddr, srcAddr, 2);
}

void GenericMessage::copyTypeAndAddr(byte* msg)
{
    msg[0] = this->type;
    msg[1] = this->srcAddr[0];
    msg[2] = this->srcAddr[1];
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
JoinAck::JoinAck(byte* srcAddr, byte hopsToGateway) : GenericMessage(MESSAGE_JOIN_ACK, srcAddr)
{
    this->hopsToGateway = hopsToGateway;
}

int JoinAck::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    byte msg[MSG_LEN_JOIN_ACK];
    copyTypeAndAddr(msg);
    msg[3] = this->hopsToGateway;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------JoinCFM Message-------------------*/
JoinCFM::JoinCFM(byte* srcAddr, byte depth) : GenericMessage(MESSAGE_JOIN_CFM, srcAddr)
{
    this->depth = depth;
}

int JoinCFM::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    byte msg[MSG_LEN_JOIN_CFM];
    copyTypeAndAddr(msg);
    msg[3] = this->depth;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------CheckAlive Message-------------------*/
CheckAlive::CheckAlive(byte* srcAddr, byte depth) : GenericMessage(MESSAGE_CHECK_ALIVE, srcAddr)
{
    this->depth = depth;
}

int CheckAlive::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    byte msg[MSG_LEN_CHECK_ALIVE];
    copyTypeAndAddr(msg);
    msg[3] = this->depth;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}


/*--------------------ReplyAlive Message-------------------*/
ReplyAlive::ReplyAlive(byte* srcAddr) : GenericMessage(MESSAGE_REPLY_ALIVE, srcAddr){
  
}

/*--------------------GatewayRequest Message-------------------*/
GatewayRequest::GatewayRequest(byte* srcAddr, byte seqNum): GenericMessage(MESSAGE_GATEWAY_REQ, srcAddr)
{
    this->seqNum = seqNum;
}

int GatewayRequest::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    byte msg[MSG_LEN_GATEWAY_REQ];
    copyTypeAndAddr(msg);
    msg[3] = this->seqNum;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------NodeReply Message-------------------*/
NodeReply::NodeReply(byte* srcAddr, byte numOfNodes, byte seqNum, 
                byte dataLength, byte* data) : GenericMessage(MESSAGE_NODE_REPLY, srcAddr)
{
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

    byte msg[this->dataLength + 6];
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

            byte hopsToGateway = buff[2];

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

            byte depth = buff[2];

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

            byte depth = buff[2];

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

            byte depth = buff[2];

            msg = new GatewayRequest(srcAddr, depth);
            delete[] buff;
            break;
        }

        case MESSAGE_NODE_REPLY:
        {
            // we have already read the msg type
            // need to know the data length before getting the data
            // TODO: check available before reading from device (else will receive 0xFF if nothing in buffer)
            byte srcAddr[2];
            srcAddr[0] = driver->recv();
            srcAddr[1] = driver->recv();

            byte numOfNodes = driver->recv();
            byte seqNum = driver->recv();
            byte dataLength = driver->recv();

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
    
    while(i < msgLen)
    {
        if(driver->available())
        {
            byte c = driver->recv();
            buff[i] = c;
            Serial.print(buff[i], HEX);
            Serial.print(" ");
            i++;
        }   
        // if(c == 0xFF){
        //     continue;
        // }        
    }
    Serial.print("\n");
    return buff;
}

/* may be deleted later
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
}*/

