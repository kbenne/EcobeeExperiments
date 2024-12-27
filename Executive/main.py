import requests
import json
import time
import serial
import threading

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
                print(f"Received from thermostat: {data}")  # Process the data (e.g., print it)
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
            "start_time": 0,
            "warmup_period": 0
        })
    )
    
    zone_temp = response.json()['payload']['read_TRoomTemp_y']
    print(f"zone_temp: {zone_temp}")

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

    while response.status_code == 200:
        if time.time() - t >= ADVANCE_INTERVAL:
            t = t + ADVANCE_INTERVAL

            json_data = json.dumps({
                    "overwrite_FurnaceStatus_u": f"{heating_status}",
                    "overwrite_FurnaceStatus_activate": f"{ON}",
                    "overwrite_ACstatus_u": f"{cooling_status}",
                    "overwrite_ACstatus_activate": f"{ON}"
                })

            print(f"Sent to BOPTEST: {json_data}")  # Process the data (e.g., print it)

            response = requests.post(
                url=f"http://{boptest_host}/advance/{testid}",
                headers={
                    "Content-Type": "application/json; charset=utf-8",
                },
                data = json_data
            )

            zone_temp = response.json()['payload']['read_TRoomTemp_y'] - 273.15
            payload = {"temperature": zone_temp}
            serialio.write(json.dumps(payload).encode('utf-8'))
            
            print(f"Sent to thermostat: {payload}")

except KeyboardInterrupt:
    print('Stopping')

finally:
    response = requests.put(
        url=f"http://{boptest_host}/stop/{testid}",
    )
    serialio.stop()
    print('Stopped')

