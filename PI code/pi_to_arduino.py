import serial
import time
import datetime


colors = ("RED","WHITE","BLUE","YELLOW","GREEN","BLACK")
words = ("ABORT","DETONATE","HOLD","PRESS")
now_plus_5min = None
class dummy_port:
    """a dummy port for automated testing"""
    def __init__(self):
        self.in_buffer = b''
        self.out_buffer = b''

        
    def read(self,n):
        data = self.in_buffer[:n]
        self.in_buffer = self.in_buffer[n:]
        return data

    
    def write(self,data):
        if not isinstance(data,bytes):
            raise TypeError("I CAN ONLY EAT BYTES")
        self.out_buffer += data

        
    def get_in_buffer(self):
        return self.in_buffer

    
    def set_in_buffer(self,new_buffer):
        self.in_buffer = new_buffer

        
    def get_out_buffer(self):
        return self.out_buffer

    
    def set_out_buffer(self,new_buffer):
        self.out_buffer = new_buffer

        
def setup(port):
    time.sleep(2)
    port.write(b"\x02\x02")
    port.write(b"\x03\x03\x01")
    global now_plus_5min
    now_plus_5min = datetime.datetime.now() + datetime.timedelta(minutes=5)


def loop(port):
    setup(port)
    buffer = b''
    last_display = 0
    while True:
        buffer,last_display = read_and_parse(port,buffer,last_display)


def read_and_parse(port,buffer,last_display):
    buffer += port.read(10)
    buffer,last_display = parse_message(port,buffer,last_display)
    return buffer,last_display


def parse_message(port,buffer,last_display):
    length = buffer[0] if buffer else 0
    if len(buffer) >= length and length != 0:
        message_type = buffer[1]
        print(f"length: {length}, type: {message_type}")
        if message_type == 0x0c:
            time_remain = now_plus_5min - datetime.datetime.now()
            mins = time_remain.seconds // 60
            seconds = time_remain.seconds % 60
            port.write(b'\x04\x05')
            #port.write(bytes([mins]))
            print(mins)
            port.write(bytes([int(str(seconds),16)]))
        elif message_type == 0x0f:
            D = input("Input number of D batteries: ")
            AA = input("Input number of AA batteries: ")
            port.write(b'\x03\x08')
            port.write(bytes([int(AA+D,16)]))
            port.write(b"\x02\x04")
        elif message_type == 0x0e:
            indicators = ('CAR', 'CLR', 'FRK', 'FRQ', 'NSA', 'MSA', 'BOB', 'TRN')
            byte = 0
            for i,indicator in enumerate(indicators):
                has_ind = input(f"do you want a {indicator} indicator(anything = yes, empty = no): ") # noqa: E501
                byte += bool(has_ind) << i
            port.write(b'\x03\x07')
            port.write(bytes([int(byte)]))
        elif message_type == 0x0b:
            print(words[buffer[3]],colors[buffer[4]],colors[buffer[5]])
        elif length:
            print(f"DATA: {buffer[length-2]}")
        buffer = buffer[length:]
    time_remain = now_plus_5min - datetime.datetime.now()
    if time.time() - last_display > 0.3:
        print(str(time_remain)[:10])
        last_display = time.time()
    return buffer,last_display


if __name__ == '__main__':
    port_num = 4
    port = serial.Serial(f"COM{port_num}",timeout=0)
    loop(port)
