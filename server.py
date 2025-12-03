from flask import Flask, request, jsonify
from flask_cors import CORS
import serial
import serial.tools.list_ports
import time
from datetime import datetime

app = Flask(__name__)
CORS(app)

# Arduino connection
arduino = None
BAUD_RATE = 9600

def find_arduino_port():
    """Automatically find Arduino port"""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'Arduino' in port.description or 'CH340' in port.description or 'USB' in port.description:
            return port.device
    return None

def connect_arduino():
    """Connect to Arduino"""
    global arduino
    try:
        port = find_arduino_port()
        if port:
            arduino = serial.Serial(port, BAUD_RATE, timeout=1)
            time.sleep(2)
            print(f"Connected to Arduino on {port}")
            return True
        else:
            print("Arduino not found")
            return False
    except Exception as e:
        print(f"Error: {e}")
        return False

def send_to_arduino(command):
    """Send command to Arduino"""
    global arduino
    try:
        if arduino is None or not arduino.is_open:
            if not connect_arduino():
                return False
        
        arduino.write(f"{command}\n".encode())
        time.sleep(0.1)
        
        if arduino.in_waiting > 0:
            response = arduino.readline().decode().strip()
            print(f"Arduino: {response}")
        
        return True
    except Exception as e:
        print(f"Error: {e}")
        arduino = None
        return False

@app.route('/')
def index():
    return jsonify({
        "status": "Server Running",
        "arduino_connected": arduino is not None and arduino.is_open
    })

@app.route('/command', methods=['GET'])
def command():
    cmd = request.args.get('cmd', '')
    
    if cmd not in ['water', 'fertilize']:
        return jsonify({"error": "Invalid command"}), 400
    
    if send_to_arduino(cmd.upper()):
        return jsonify({"success": True, "message": f"{cmd} command sent"})
    else:
        return jsonify({"success": False, "error": "Arduino communication failed"}), 500

@app.route('/setdate', methods=['GET'])
def set_date():
    date = request.args.get('date', '')
    time_str = request.args.get('time', '08:00')
    
    if not date:
        return jsonify({"error": "Date required"}), 400
    
    arduino_cmd = f"SCHEDULE:{date}:{time_str}"
    
    if send_to_arduino(arduino_cmd):
        return jsonify({"success": True, "date": date, "time": time_str})
    else:
        return jsonify({"success": False, "error": "Arduino communication failed"}), 500

@app.route('/status', methods=['GET'])
def status():
    return jsonify({
        "arduino_connected": arduino is not None and arduino.is_open,
        "server_time": datetime.now().isoformat()
    })

if __name__ == '__main__':
    print("Garden Watering System Server")
    print("Connecting to Arduino...")
    connect_arduino()
    print("Server starting on http://localhost:8000")
    app.run(host='0.0.0.0', port=8000, debug=True)
