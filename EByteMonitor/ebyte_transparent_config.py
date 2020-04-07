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


  
