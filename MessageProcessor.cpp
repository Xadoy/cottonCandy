#include "MessageProcessor.h"
#include <string.h>

/*
Note:
Little Endian reverse the order by bytes
e.g. 
addr = 0x1234 
addr[0] = 34 addr[1] = 12
*/

void GenericMessage::copyTypeAndAddr(char* msg)
{
  msg[0] = this->type;
  //reverse the bytes
  msg[1] = ((unsigned char *)&(this->addr))[1];
  msg[2] = ((unsigned char *)&(this->addr))[0];
}


void GenericMessage::send()
{
  char msg[3]; 
  copyTypeAndAddr(msg);

  //send out using device driver
}

void CheckAlive::send()
{
  char msg[4];
  copyTypeAndAddr(msg);

  msg[3] = this->depth;

  //send out using device driver
}

void JoinConfirm::send()
{
  char msg[4];
  copyTypeAndAddr(msg);

  msg[3] = this->depth;

  //send out using device driver
}

void GatewayRequest::send()
{
  char msg[4];
  copyTypeAndAddr(msg);

  msg[3] = this->seqNum;

  //send out using device driver
}

void NodeReply::send()
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



