import requests
import json
import time
import serial
import threading
import time
from datetime import datetime, timedelta

boptest_host = '10.1.1.158'
testcase_id = 'g1700430'

STEP_SIZE = 30.0
TIME_SCALER = 15.0
ON = 1
OFF = 0
fan_status = 0
heating_status = 0
cooling_status = 0
ADVANCE_INTERVAL = STEP_SIZE / TIME_SCALER
serial_port = '/dev/tty.usbserial-0001'
baudrate = 115200

# The epoch from the BOPTEST / Modelica / Spawn point of view
epoch_datetime = datetime(year=2024, month=1, day=1, hour=0, minute=0, second=0)
start_datetime = datetime(year=2024, month=1, day=1, hour=0, minute=0, second=0)
start_seconds = (start_datetime - epoch_datetime).total_seconds()

def on_off_str(val):
    if val:
        return "On"
    else:
        return "Off"

def kelvin_to_fahrenheit(kelvin):
    """
    Convert temperature from Kelvin to Fahrenheit.

    Formula: F = (K - 273.15) * 9/5 + 32

    :param kelvin: Temperature in Kelvin
    :return: Temperature in Fahrenheit
    """
    if kelvin < 0:
        raise ValueError("Temperature in Kelvin cannot be negative.")
    
    fahrenheit = (kelvin - 273.15) * 9/5 + 32
    return fahrenheit


class SerialIO:
    def __init__(self, port, baudrate):
        self.ser = serial.Serial(serial_port, baudrate)
        self.running = True
        self.thread = threading.Thread(target=self.readln)
        self.thread.start()

    def readln(self):
        global fan_status, heating_status, cooling_status
        while self.running:
            if self.ser.in_waiting > 0:  # Check if data is available
                data = self.ser.readline().decode('utf-8').rstrip()  # Read a line
                #print(f"Received from thermostat: {data}")  # Process the data (e.g., print it)
                try:
                    json_data = json.loads(data)
                    if "input0" in json_data:
                        fan_status = json_data["input0"]
                    if "input1" in json_data:
                        heating_status = json_data["input1"]
                    if "input2" in json_data:
                        cooling_status = json_data["input2"]
                except Exception as e:
                    pass


    def write(self, data):
        self.ser.write(data)

    def stop(self):
        self.running = False
        self.thread.join()  # Wait for the thread to finish
        self.ser.close()

print('Starting')

serialio = SerialIO(serial_port, baudrate)

response = requests.post(
    url=f"http://{boptest_host}/testcases/{testcase_id}/select",
)

if response.status_code != 200:
    raise Error('Could not select testcase')

testid = response.json()['testid']

print(f"testid is {testid}")

try:
    response = requests.put(
        url=f"http://{boptest_host}/initialize/{testid}",
        headers={
            "Content-Type": "application/json; charset=utf-8",
        },
        data=json.dumps({
            "start_time": start_seconds,
            "warmup_period": 0
        })
    )

    response = requests.put(
        url=f"http://{boptest_host}/step/{testid}",
        headers={
            "Content-Type": "application/json; charset=utf-8",
        },
        data=json.dumps({
            "step": STEP_SIZE,
        })
    )

    t = time.time()
    dt = start_datetime

    while response.status_code == 200:
        if time.time() - t >= ADVANCE_INTERVAL:
            t = t + ADVANCE_INTERVAL

            dt = dt + timedelta(seconds=STEP_SIZE)
            pretty_dt = dt.strftime("%A, %B %d, %Y %H:%M:%S")
            print(pretty_dt)

            json_data = json.dumps({
                    "overwrite_FurnaceStatus_u": f"{heating_status}",
                    "overwrite_FurnaceStatus_activate": f"{ON}",
                    "overwrite_ACstatus_u": f"{cooling_status * -1}",
                    "overwrite_ACstatus_activate": f"{ON}"
                })

            response = requests.post(
                url=f"http://{boptest_host}/advance/{testid}",
                headers={
                    "Content-Type": "application/json; charset=utf-8",
                },
                data = json_data
            )

            zone_temp = response.json()['payload']['read_TRoomTemp_y']
            payload = {"temperature": zone_temp - 273.15}
            serialio.write(json.dumps(payload).encode('utf-8'))
            
            print(f"Zone Temperature: {'{:.2f}'.format(kelvin_to_fahrenheit(zone_temp))}")

            oa_temp = response.json()['payload']['read_TAmb_y']
            #print(f"Outside Temperature: {'{:.2f}'.format(kelvin_to_fahrenheit(oa_temp))}")

            print(f"Heating: {on_off_str(heating_status)}, Cooling: {on_off_str(cooling_status)}, Fan: {on_off_str(fan_status)}")
            print("")

except KeyboardInterrupt:
    print('Stopping')

finally:
    response = requests.put(
        url=f"http://{boptest_host}/stop/{testid}",
    )
    serialio.stop()
    print('Stopped')

