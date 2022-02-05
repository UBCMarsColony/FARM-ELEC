import time
import serial


def main():
    ser = serial.Serial("dev/ttyS0", baudrate=9600, timeout=0.5)
    print("Starting Code")
    ser.flushInput()

    while(1):
        CO2 = readCO2(ser)
        print(str(CO2))
        time.sleep(5)


def readCO2(ser):
    ser.flushInput()
    ser.write("\xFE\x44\x00\x08\x02\x9F\x25")
    time.sleep(0.5)
    resp = ser.read(7)
    high = ord(resp[3])
    low = ord(resp[4])
    co2 = (high*256) + low
    return co2

#http://co2meters.com/Documentation/Other/AN_SE_0018_AN_137_Revised8.pdf
