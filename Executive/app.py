import requests
import json
import time
import serial
import threading
from datetime import datetime, timedelta
from flask import Flask, jsonify, request
from flask_cors import CORS
import sys

# -------------------
# Global Configuration
# -------------------
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
start_datetime = datetime(year=2024, month=1, day=1, hour=13, minute=0, second=0)
start_seconds = (start_datetime - epoch_datetime).total_seconds()

# ---------------------------------
# Global variable to track zone temp
# ---------------------------------
latest_zone_temp_kelvin = 0.0  # We'll store the raw Kelvin value here
latest_oa_temp_kelvin = 0.0 
dt = start_datetime

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

def HVAC_status():
    # Determine the HVAC status
    hvac_status = "Off"  # Default
    if heating_status:
        hvac_status = "Heat"
    elif cooling_status:
        hvac_status = "Cool"
    elif fan_status:
        hvac_status = "Fan"
    
    return hvac_status

    

# -------------------
# Flask App
# -------------------
app = Flask(__name__)
CORS(app)

@app.route('/sim_data', methods=['GET'])
def get_zone_temp():
    """
    Returns the latest zone temperature in both Kelvin and Fahrenheit.
    """
    global latest_zone_temp_kelvin, latest_oa_temp_kelvin
    return jsonify({
        'indoorTemp': kelvin_to_fahrenheit(latest_zone_temp_kelvin),
        'outdoorTemp': kelvin_to_fahrenheit(latest_oa_temp_kelvin),
        'currentDateTime': dt.strftime("%Y-%m-%d %H:%M:%S"),
        'hvacStatus': HVAC_status()
    })

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
        """
        Continuously read lines from the serial device, parse JSON,
        and update fan/heating/cooling statuses.
        """
        global fan_status, heating_status, cooling_status
        while self.running:
            if self.ser.in_waiting > 0:  # Check if data is available
                data = self.ser.readline().decode('utf-8').rstrip()  # Read a line
                try:
                    json_data = json.loads(data)
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
        """
        Send data over serial.
        """
        self.ser.write(data)

    def stop(self):
        """
        Cleanly stop reading from the serial port.
        """
        self.running = False
        self.thread.join()  # Wait for the thread to finish
        self.ser.close()

# -------------------
# Global references for simulation
# -------------------
simulation_thread = None
stop_simulation_flag = False
serialio = None
testid = None

def run_simulation():
    """
      1. Start serial
      2. Select test case
      3. Initialize test
      4. Set step size
      5. Enter main while loop
    """
    global serialio, testid, stop_simulation_flag, latest_zone_temp_kelvin, latest_oa_temp_kelvin, dt
    try:
        # 1. Start serial
        print('Starting serial connection...')
        serialio = SerialIO(serial_port, baudrate)

        # 2. Select the test case
        print('Selecting test case...')
        response = requests.post(
            url=f"http://{boptest_host}/testcases/{testcase_id}/select",
        )

        if response.status_code != 200:
            print("Could not select testcase.")
            return  # or sys.exit(1), but in a Flask thread, it's best to return

        testid = response.json()['testid']
        print(f"testid is {testid}")

        # 3. Initialize the test
        response = requests.put(
            url=f"http://{boptest_host}/initialize/{testid}",
            headers={"Content-Type": "application/json; charset=utf-8"},
            data=json.dumps({
                "start_time": start_seconds,
                "warmup_period": 0
            })
        )

        # 4. Set step size
        response = requests.put(
            url=f"http://{boptest_host}/step/{testid}",
            headers={"Content-Type": "application/json; charset=utf-8"},
            data=json.dumps({
                "step": STEP_SIZE,
            })
        )

        t = time.time()
        dt = start_datetime

        # 5. Main control loop
        while response.status_code == 200 and not stop_simulation_flag:
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
                latest_zone_temp_kelvin = zone_temp_kelvin  # update global

                # Send data to serial (in Celsius)
                payload = {"temperature": zone_temp_kelvin - 273.15}
                serialio.write(json.dumps(payload).encode('utf-8'))

                # Print info
                print(f"Zone Temperature (F): {kelvin_to_fahrenheit(zone_temp_kelvin):.2f}")
                latest_oa_temp_kelvin = response.json()['payload']['read_TAmb_y']
                print(f"Outside Temperature (F): {kelvin_to_fahrenheit(latest_oa_temp_kelvin):.2f}")
                print(f"Heating: {on_off_str(heating_status)}, Cooling: {on_off_str(cooling_status)}, Fan: {on_off_str(fan_status)}")
                print("")

        # Once we exit the loop, either stop_simulation_flag == True or error
    except Exception as e:
        print(f'Error in run_simulation: {e}')
    finally:
        # Stop the test case if it was started
        if testid is not None:
            print('Stopping test case...')
            requests.put(url=f"http://{boptest_host}/stop/{testid}",)

        # Stop serial if started
        if serialio is not None:
            serialio.stop()

        print('Stopped simulation thread.')

@app.route('/api/start_simulation', methods=['POST'])
def start_simulation():
    """
    Endpoint to start the entire simulation in a background thread.
    """
    global simulation_thread, stop_simulation_flag

    # If you want to guard against multiple starts:
    if simulation_thread and simulation_thread.is_alive():
        return jsonify({"message": "Simulation is already running"}), 400

    stop_simulation_flag = False
    simulation_thread = threading.Thread(target=run_simulation, daemon=True)
    simulation_thread.start()
    return jsonify({"message": "Simulation started"}), 200

@app.route('/api/stop_simulation', methods=['POST'])
def stop_simulation():
    """
    Optional endpoint to signal the simulation loop to stop gracefully.
    """
    global stop_simulation_flag
    stop_simulation_flag = True
    return jsonify({"message": "Stop signal sent"}), 200

# Only run the Flask server if this file is called directly:
# We no longer do the entire simulation in the main block;
# we just start the server, and wait for /api/start_simulation calls.
if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=False, use_reloader=False)
