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
