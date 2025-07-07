import serial
import time

GPS_PORT = "/dev/ttyUSB0"
ESP32_PORT = "/dev/ttyUSB1"

gps_serial = serial.Serial(GPS_PORT, 9600, timeout=1)

esp_serial = serial.Serial(ESP32_PORT, 115200, timeout=1)

try:
    while True:
        if gps_serial.in_waiting:
            line = gps_serial.readline()
            print("→ GPS:", line.decode(errors='ignore').strip())
            esp_serial.write(line)
        time.sleep(0.01)
except KeyboardInterrupt:
    print("Finishing...")
finally:
    gps_serial.close()
    esp_serial.close()