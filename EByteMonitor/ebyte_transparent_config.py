'''   
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
'''

'''
This is a utility Python script for configuring Ebyte E22 transceiver. It
assumes that the E22 transceiver is connected directly to the PC through
USB port and is in configuration mode. The script sends a command to the
tranceiver and receives a reply to set the register in the Ebyte device.
'''

import serial
import time
import json
from datetime import datetime

command=bytearray([0xC1, 0x03, 0x01])
print("Try to open serial port")

with serial.Serial('/dev/cu.wchusbserial1410', 9600, timeout=5) as ser:
  print("Write command to the port")
  ser.write(command)
  s=ser.read(7)
  print(s)


  
