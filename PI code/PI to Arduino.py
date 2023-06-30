import serial
import time

port_num = 4
port = serial.Serial(f"COM{port_num}",timeout=5)

time.sleep(2)
port.write(b"\x02\x02")
port.write(b"\x03\x03\x01")
port.write(b"\x02\x04")
