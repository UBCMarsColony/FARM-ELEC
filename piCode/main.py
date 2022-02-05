import serial
import csv
PATH = "/media/usb/file.csv"


def writeToCSV(string):
    f = open(PATH)
    writer = csv.writer(f)
    writer.writerow(string)
    f.close()
    
if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    ser.reset_input_buffer()
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            writeToCSV(line)


