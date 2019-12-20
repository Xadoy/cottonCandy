#include "MessageProcessor.h"
#include <string.h>


GenericMessage::GenericMessage(unsigned char type, address srcAddr)
{
  this->type = type;
  this->srcAddr = srcAddr;
}

//Note:Little Endian reverse the order of bytes

void GenericMessage::copyTypeAndAddr(char* msg)
{
  msg[0] = this->type;
  //reverse the bytes
  msg[1] = ((unsigned char *)&(this->srcAddr))[1];
  msg[2] = ((unsigned char *)&(this->srcAddr))[0];
}


int GenericMessage::send(DeviceDriver* driver, address destAddr)
{
  char msg[3]; 
  copyTypeAndAddr(msg);

  //send out using device driver
}

CheckAlive::CheckAlive(unsigned char type, address srcAddr, unsigned char depth)
{
  this->type = type;
  this->srcAddr = srcAddr;
  this->depth = depth;
}

int CheckAlive::send(DeviceDriver* driver, address destAddr)
{
  char msg[4];
  copyTypeAndAddr(msg);

  msg[3] = this->depth;

  //send out using device driver
}

JoinConfirm::JoinConfirm(unsigned char type, address srcAddr, unsigned char depth)
{
  this->type = type;
  this->srcAddr = srcAddr;
  this->depth = depth;
}

int JoinConfirm::send(DeviceDriver* driver, address destAddr)
{
  char msg[4];
  copyTypeAndAddr(msg);

  msg[3] = this->depth;

  //send out using device driver
}

GatewayRequest::GatewayRequest(unsigned char type, address srcAddr, unsigned char seqNum)
{
  this->type = type;
  this->srcAddr = srcAddr;
  this->seqNum = seqNum;
}

int GatewayRequest::send(DeviceDriver* driver, address destAddr)
{
  char msg[4];
  copyTypeAndAddr(msg);

  msg[3] = this->seqNum;

  //send out using device driver
}

NodeReply::NodeReply(unsigned char type, address srcAddr, unsigned char numOfNodes, unsigned char seqNum, 
                unsigned char dataLength, char data[8])
{
  this->type = type;
  this->srcAddr = srcAddr;
  this->numOfNodes = numOfNodes;
  this->seqNum = seqNum;
  this->dataLength = dataLength;
  strcpy(this->data, data);
}

int NodeReply::send(DeviceDriver* driver, address destAddr)
{
  char msg[14];
  copyTypeAndAddr(msg);

  msg[3] = this->numOfNodes;
  msg[4] = this->seqNum;
  msg[5] = this->dataLength;

  for(int i = 0; i < 8; i++)
  {
    msg[6 + i] = data[i];
  }

  //send out using device driver
}



