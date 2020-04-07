#include "LoRaMesh.h"

LoRaMesh::LoRaMesh(byte *addr, DeviceDriver *driver)
{
  myEngine = new ForwardEngine(addr,driver);
}

LoRaMesh::~LoRaMesh()
{
  if (myEngine != nullptr)
  {
    delete myEngine;
  }
}

void LoRaMesh::setAddr(byte *addr)
{
   myEngine->setAddr(addr);
}

byte *LoRaMesh::getMyAddr()
{
  return myEngine->getMyAddr();
}

byte *LoRaMesh::getParentAddr()
{
  return myEngine->getParentAddr();
}

void LoRaMesh::setGatewayReqTime(unsigned long gatewayReqTime)
{
  myEngine->setGatewayReqTime(gatewayReqTime);
}

unsigned long LoRaMesh::getGatewayReqTime()
{
  return myEngine->getGatewayReqTime();
}

void LoRaMesh::onReceiveRequest(void(*callback)(byte**, byte*)) {
  myEngine->onReceiveRequest(callback);
}
void LoRaMesh::onReceiveResponse(void(*callback)(byte*, byte)) {
  myEngine->onReceiveResponse(callback);
}

bool LoRaMesh::join()
{
  return myEngine->join();
}

bool LoRaMesh::run()
{
  return myEngine->run();
}
