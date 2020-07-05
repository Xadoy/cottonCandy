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

#ifndef HEADER_LORA_MESH
#define HEADER_LORA_MESH

#include "ForwardEngine.h"

class LoRaMesh{

public:
    /**
     * Destructor
     */
    ~LoRaMesh();

    /**
     * Constructor. Requires driver and assigned addr
     */
    LoRaMesh(byte* addr, DeviceDriver* driver);

    /**
     * Try to join an existing network by finding a parent. Return true if successfully joined an 
     * existing network
     */
    bool join();

    /**
     * Node exits an existing network
     */
    void disconnect();

    /**
     * This is the core loop where the node operates sending and receiving after it  joined the network.
     * Returns true if the node is started successfully.
     * 
     * Note: This method can be used without calling join() prior. In this case, the node assumes that
     * there is no existing network and it is the first node in the new network. 
     */
    bool run();


    //Setter for the node address
    void setAddr(byte* addr);

    /**
     * Getter for the node address
     */
    byte* getMyAddr();

    /**
     * Getter for the parent address
     */
    byte* getParentAddr();

    /**
     * Setter for the time interval between each GatewayRequest 
     */
    void setGatewayReqTime(unsigned long gatewayReqTime);

    /**
     * Getter for the time interval between each GatewayRequest 
     */ 
    unsigned long getGatewayReqTime();

    /**
     * Accepts a function as an argument which will be called when a gateway request arrives
     */
    void onReceiveRequest(void(*callback)(byte**, byte*));

    /**
     * Accepts a function as an argument which will be called when a node reply arrives
     */
    void onReceiveResponse(void(*callback)(byte*, byte, byte*));


private:

  ForwardEngine* myEngine;

};

#endif
