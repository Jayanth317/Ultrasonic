# Ultrasonic Sensor for distance
The sensor used in this example is DYP-A02-V2.1
The series model is DYP-A02YYU-V2.0 
It is a Split waterproof sensor and works on UART auto output.

# UART output format

Frame  data  |  Description                 | Byte

Frame header | Fixed to 0xFF                | 1 byte

Data_H       | High 8 bits of distance data | 1 byte

Data_L       | Low 8 bits of distance data  | 1 byte

SUM          | Communication checksum       | 1 byte

# UART output example

Frame header  | Data_H |  Data_L | SUM

    0XFF      |  0X07  |  0XA1   | 0XA7
    
Note: The checksum only retains the lower 8 bits of the accumulated value;

SUM = (Frame header + Data_H + Data_L)&0x00FF

=(0XFF + 0X07 + 0XA1)&0x00FF

=0XA7;


Distance value = Data_H*256+ Data_L=0X07A1;

Converted to decimal is equal to 1953;

Indicates that the currently measured distance value is 1953 mm.
