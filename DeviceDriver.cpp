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

#include "DeviceDriver.h"

DeviceDriver::DeviceDriver(){

}

DeviceDriver::~DeviceDriver(){

}

bool DeviceDriver::init(){
    return true;
}

int DeviceDriver::send(byte* destAddr, byte* msg, long msgLen){
    Serial.println("Send not implemented in this dummy driver");
    return -1;
}

byte DeviceDriver::recv(){
    Serial.println("Recv not implemented in this dummy driver");
    return -1;
}
