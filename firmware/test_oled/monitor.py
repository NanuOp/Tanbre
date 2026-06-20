import serial
import time

s = serial.Serial('/dev/ttyACM0', 115200, timeout=0.5)

# Reset device first to see clean output
s.dtr = False
s.rts = True
time.sleep(0.1)
s.dtr = True
s.rts = False
time.sleep(0.1)
s.dtr = True
s.rts = True

print("Monitoring started. Please check and wiggle the wires...")
start_time = time.time()
while time.time() - start_time < 45:
    line = s.readline().decode('utf-8', errors='ignore').strip()
    if line:
        print(line)
    time.sleep(0.01)

print("Monitoring finished.")
s.close()
