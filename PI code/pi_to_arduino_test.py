import pi_to_arduino
import pytest
class TestClass:
    def test_dummy(self):
        port = pi_to_arduino.dummy_port()
        pi_to_arduino.setup(port)
        buffer = b''
        last_display = 0
        pi_to_arduino.read_and_parse(port,buffer,last_display)
    def test_setup(self):
        port = pi_to_arduino.dummy_port()
        pi_to_arduino.setup(port)
        assert b'\x02\x02\x03\x03' in port.out_buffer
    def test_get_time(self):
        port = pi_to_arduino.dummy_port()
        pi_to_arduino.setup(port)
        buffer = b''
        port.set_in_buffer(b'\x02\x0c')
        last_display = 0
        pi_to_arduino.read_and_parse(port,buffer,last_display)
        port.set_out_buffer(port.out_buffer[5:])
        assert b'\x04\x05' in port.out_buffer
        assert len(port.out_buffer) == 4
        
