/*    
    Copyright 2020, Network Research Lab at the University of Toronto.

    This file is part of CottonCandy.

    CottonCandy is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CottonCandy is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CottonCandy.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "MessageProcessor.h"
#include <string.h>
#include "Utilities.h"


GenericMessage::GenericMessage(byte type, byte* srcAddr, byte* destAddr)
{
    this->type = type;  
    
    memcpy(this->srcAddr, srcAddr, 2);
    memcpy(this->destAddr, destAddr, 2);
}

void GenericMessage::copyTypeAndAddr(byte* msg)
{
    msg[0] = this->type;
    msg[1] = this->srcAddr[0];
    msg[2] = this->srcAddr[1];
    msg[3] = this->destAddr[0];
    msg[4] = this->destAddr[1];
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
}

/*--------------------Join Beacon-------------------*/
Join::Join(byte* srcAddr, byte* destAddr) : GenericMessage(MESSAGE_JOIN, srcAddr, destAddr)
{
}


/*--------------------JoinACK Message-------------------*/
JoinAck::JoinAck(byte* srcAddr, byte* destAddr, byte hopsToGateway) : GenericMessage(MESSAGE_JOIN_ACK, srcAddr, destAddr)
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
    msg[5] = hopsToGateway;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------JoinCFM Message-------------------*/
JoinCFM::JoinCFM(byte* srcAddr, byte* destAddr, byte depth) : GenericMessage(MESSAGE_JOIN_CFM, srcAddr, destAddr)
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
    msg[5] = depth;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------CheckAlive Message-------------------*/
CheckAlive::CheckAlive(byte* srcAddr, byte* destAddr, byte depth) : GenericMessage(MESSAGE_CHECK_ALIVE, srcAddr, destAddr)
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
    msg[5] = depth;

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}


/*--------------------ReplyAlive Message-------------------*/
ReplyAlive::ReplyAlive(byte* srcAddr, byte* destAddr) : GenericMessage(MESSAGE_REPLY_ALIVE, srcAddr, destAddr)
{
  
}

/*--------------------GatewayRequest Message-------------------*/
GatewayRequest::GatewayRequest(byte* srcAddr, byte* destAddr, byte seqNum, unsigned long nextReqTime, unsigned long childBackoffTime): GenericMessage(MESSAGE_GATEWAY_REQ, srcAddr, destAddr)
{
    this->seqNum = seqNum;
    this->nextReqTime = nextReqTime;
    this->childBackoffTime = childBackoffTime;
}

int GatewayRequest::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    byte msg[MSG_LEN_GATEWAY_REQ];
    copyTypeAndAddr(msg);
    msg[5] = seqNum;

    union LongConverter converter;

    /**
     * Note that for transmitting type Long, we used C union for 
     * converting a Long-type variable to a 4-byte array. The byte 
     * order used in the transmission is in little endian.
     * 
     * TODO: An enhancement would be converting the byte order to the
     * typical network order (i.e. Big endian), but it is not a priority. 
     */ 
    converter.l = nextReqTime;
    memcpy(&(msg[6]), converter.b, sizeof(converter.b));

    converter.l = childBackoffTime;
    memcpy(&(msg[10]), converter.b, sizeof(converter.b));

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

/*--------------------NodeReply Message-------------------*/
NodeReply::NodeReply(byte* srcAddr, byte* destAddr, byte seqNum, 
                byte dataLength, byte* data) : GenericMessage(MESSAGE_NODE_REPLY, srcAddr, destAddr)
{
    this->seqNum = seqNum;
    this->dataLength = dataLength;
    this->data = new byte[dataLength];
    memcpy(this->data, data, dataLength);
}
NodeReply::~NodeReply() {
    delete this->data;
}

int NodeReply::send(DeviceDriver* driver, byte* destAddr)
{
    if(driver == NULL)
    {
        return -1;
    }

    byte msg[dataLength + MSG_LEN_HEADER_NODE_REPLY];
    copyTypeAndAddr(msg);

    msg[5] = seqNum;
    msg[6] = dataLength;
    memmove(msg + 7, data, dataLength);

    return ( driver->send(destAddr, msg, sizeof(msg)) );
}

GenericMessage* receiveMessage(DeviceDriver* driver, unsigned long timeout)
{
    unsigned long startTime = getTimeMillis();
    char msgType = -1;
    GenericMessage* msg = nullptr;

    while((unsigned long)(getTimeMillis() - startTime) < timeout)
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
            byte* buff = readMsgFromBuff(driver, MSG_LEN_JOIN - 1, timeout);

            // get what we need for Join
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);
            byte destAddr[2];
            memcpy(destAddr, buff + 2, 2);

            msg = new Join(srcAddr, destAddr);
            delete[] buff;
            break;
        }        

        case MESSAGE_JOIN_ACK:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_JOIN_ACK - 1, timeout);

            // get what we need for JoinAck
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);
            byte destAddr[2];
            memcpy(destAddr, buff + 2, 2);

            byte hopsToGateway = buff[4];

            msg = new JoinAck(srcAddr, destAddr, hopsToGateway);
            delete[] buff;
            break;
        }

        case MESSAGE_JOIN_CFM:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_JOIN_CFM - 1, timeout);

            // get what we need for JoinCFM
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);
            byte destAddr[2];
            memcpy(destAddr, buff + 2, 2);

            byte depth = buff[4];

            msg = new JoinCFM(srcAddr, destAddr, depth);
            delete[] buff;
            break;
        }

        case MESSAGE_CHECK_ALIVE:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_CHECK_ALIVE - 1, timeout);

            // get what we need for CheckAlive
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);
            byte destAddr[2];
            memcpy(destAddr, buff + 2, 2);

            byte depth = buff[4];

            msg = new CheckAlive(srcAddr, destAddr, depth);
            delete[] buff;
            break;
        }

        case MESSAGE_REPLY_ALIVE:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_REPLY_ALIVE - 1, timeout);

            // get what we need for ReplyAlive
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);
            byte destAddr[2];
            memcpy(destAddr, buff + 2, 2);

            msg = new ReplyAlive(srcAddr, destAddr);
            delete[] buff;
            break;
        }

        case MESSAGE_GATEWAY_REQ:
        {
            // we have already read the msg type
            byte* buff = readMsgFromBuff(driver, MSG_LEN_GATEWAY_REQ - 1, timeout);

            // get what we need for GatewayRequest
            byte srcAddr[2];
            memcpy(srcAddr, buff, 2);
            byte destAddr[2];
            memcpy(destAddr, buff + 2, 2);

            byte seqNum = buff[4];

            union LongConverter converter;
            memcpy(converter.b, buff + 5, 4);
            unsigned long nextReqTime = converter.l;

            memcpy(converter.b, buff + 9, 4);
            unsigned long childBackoffTime = converter.l;

            msg = new GatewayRequest(srcAddr, destAddr, seqNum, nextReqTime, childBackoffTime);
            delete[] buff;
            break;
        }

        case MESSAGE_NODE_REPLY:
        {
            // we have already read the msg type
            // need to know the data length before getting the data

            // get Header first
            byte* headerBuff = readMsgFromBuff(driver, MSG_LEN_HEADER_NODE_REPLY - 1, timeout);
            byte srcAddr[2];
            memcpy(srcAddr, headerBuff, 2);
            byte destAddr[2];
            memcpy(destAddr, headerBuff + 2, 2);

            byte seqNum = headerBuff[4];
            byte dataLength = headerBuff[5];

            byte* data = readMsgFromBuff(driver, dataLength, timeout);

            msg = new NodeReply(srcAddr, destAddr, seqNum, dataLength, data);
            delete[] data;
            delete[] headerBuff;
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
byte* readMsgFromBuff(DeviceDriver* driver, uint8_t msgLen, unsigned long timeout)
{
    byte* buff = new byte[msgLen];

    int i = 0;

    unsigned long startTime = getTimeMillis();
    
    while(i < msgLen && (unsigned long)(getTimeMillis() - startTime) < timeout)
    {
        if(driver->available())
        {
            byte c = driver->recv();
            buff[i] = c;
            // Serial.print(buff[i], HEX);
            // Serial.print(" _ ");
            i++;
        }   
        // if(c == 0xFF){
        //     continue;
        // }        
    }
    return buff;
}

