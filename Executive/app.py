import requests
import json
import time
import serial
import threading
import time
from datetime import datetime, timedelta

# -------------------
# New imports for Flask
# -------------------
from flask import Flask, jsonify
import sys

boptest_host = '172.17.0.1'
testcase_id = '31383'

STEP_SIZE = 30.0
TIME_SCALER = 15.0
ON = 1
OFF = 0
fan_status = 0
heating_status = 0
cooling_status = 0
ADVANCE_INTERVAL = STEP_SIZE / TIME_SCALER
serial_port = '/tmp/virtual-serial'
baudrate = 115200

# The epoch from the BOPTEST / Modelica / Spawn point of view
epoch_datetime = datetime(year=2024, month=1, day=1, hour=0, minute=0, second=0)
start_datetime = datetime(year=2024, month=7, day=1, hour=13, minute=0, second=0)
start_seconds = (start_datetime - epoch_datetime).total_seconds()

# ---------------------------------
# Global variable to track zone temp
# ---------------------------------
latest_zone_temp_kelvin = 0.0  # We'll store the raw Kelvin value here

# -------------------
# Helper functions
# -------------------
def on_off_str(val):
    return "On" if val else "Off"

def kelvin_to_fahrenheit(kelvin):
    """
    Convert temperature from Kelvin to Fahrenheit.
    Formula: F = (K - 273.15) * 9/5 + 32
    """
    if kelvin < 0:
        raise ValueError("Temperature in Kelvin cannot be negative.")
    fahrenheit = (kelvin - 273.15) * 9/5 + 32
    return fahrenheit

# -------------------
# Flask App
# -------------------
app = Flask(__name__)

@app.route('/zone_temp', methods=['GET'])
def get_zone_temp():
    """
    Returns the latest zone temperature in both Kelvin and Fahrenheit.
    """
    global latest_zone_temp_kelvin
    return jsonify({
        'kelvin': latest_zone_temp_kelvin,
        'fahrenheit': kelvin_to_fahrenheit(latest_zone_temp_kelvin)
    })

def run_flask_app():
    """
    Runs Flask in a separate thread so it doesn't block the main loop.
    """
    # You can change host/port as needed
    app.run(host='0.0.0.0', port=5000, debug=False, use_reloader=False)

# -------------------
# Serial I/O class
# -------------------
class SerialIO:
    def __init__(self, port, baudrate):
        self.ser = serial.Serial(port, baudrate)
        self.running = True
        self.thread = threading.Thread(target=self.readln)
        self.thread.start()
        print("Serial connection started")

    def readln(self):
        global fan_status, heating_status, cooling_status
        while self.running:
            if self.ser.in_waiting > 0:  # Check if data is available
                data = self.ser.readline().decode('utf-8').rstrip()  # Read a line
                try:
                    json_data = json.loads(data)
                    print(json_data)
                    if "input0" in json_data:
                        fan_status = json_data["input0"]
                    if "input1" in json_data:
                        heating_status = json_data["input1"]
                    if "input2" in json_data:
                        cooling_status = json_data["input2"]
                except Exception:
                    # If parsing fails, ignore it
                    pass

    def write(self, data):
        self.ser.write(data)

    def stop(self):
        self.running = False
        self.thread.join()  # Wait for the thread to finish
        self.ser.close()


# -------------------
# Main code
# -------------------
if __name__ == "__main__":

    # Start Flask in a separate thread
    flask_thread = threading.Thread(target=run_flask_app, daemon=True)
    flask_thread.start()

    print('Starting serial connection...')
    serialio = SerialIO(serial_port, baudrate)

    print('Selecting test case...')
    response = requests.post(
        url=f"http://{boptest_host}/testcases/{testcase_id}/select",
    )

    if response.status_code != 200:
        print("Could not select testcase.")
        sys.exit(1)

    testid = response.json()['testid']
    print(f"testid is {testid}")

    try:
        # Initialize the test
        response = requests.put(
            url=f"http://{boptest_host}/initialize/{testid}",
            headers={"Content-Type": "application/json; charset=utf-8"},
            data=json.dumps({
                "start_time": start_seconds,
                "warmup_period": 0
            })
        )

        # Set step size
        response = requests.put(
            url=f"http://{boptest_host}/step/{testid}",
            headers={"Content-Type": "application/json; charset=utf-8"},
            data=json.dumps({
                "step": STEP_SIZE,
            })
        )

        t = time.time()
        dt = start_datetime

        # Run control loop
        while response.status_code == 200:
            if time.time() - t >= ADVANCE_INTERVAL:
                t = t + ADVANCE_INTERVAL
                dt = dt + timedelta(seconds=STEP_SIZE)
                pretty_dt = dt.strftime("%A, %B %d, %Y %H:%M:%S")
                print(pretty_dt)

                # Overwrite commands
                json_data = json.dumps({
                    "overwrite_FurnaceStatus_u": f"{heating_status}",
                    "overwrite_FurnaceStatus_activate": f"{ON}",
                    "overwrite_ACstatus_u": f"{cooling_status * -1}",
                    "overwrite_ACstatus_activate": f"{ON}"
                })

                response = requests.post(
                    url=f"http://{boptest_host}/advance/{testid}",
                    headers={"Content-Type": "application/json; charset=utf-8"},
                    data=json_data
                )

                # Extract zone temperature in Kelvin
                zone_temp_kelvin = response.json()['payload']['read_TRoomTemp_y']
                
                # Update global variable
                latest_zone_temp_kelvin = zone_temp_kelvin

                # Send data to serial (in Celsius as an example)
                payload = {"temperature": zone_temp_kelvin - 273.15}
                serialio.write(json.dumps(payload).encode('utf-8'))

                # Print info
                print(f"Zone Temperature (F): {kelvin_to_fahrenheit(zone_temp_kelvin):.2f}")
                oa_temp = response.json()['payload']['read_TAmb_y']
                print(f"Outside Temperature (F): {kelvin_to_fahrenheit(oa_temp):.2f}")
                print(f"Heating: {on_off_str(heating_status)}, Cooling: {on_off_str(cooling_status)}, Fan: {on_off_str(fan_status)}")
                print("")

    except KeyboardInterrupt:
        print('Stopping due to KeyboardInterrupt...')

    finally:
        print('Stopping test case...')
        requests.put(url=f"http://{boptest_host}/stop/{testid}",)
        
        serialio.stop()
        print('Stopped.')
