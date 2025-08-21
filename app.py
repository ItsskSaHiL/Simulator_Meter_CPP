from flask import Flask, render_template, request, jsonify, send_from_directory
import os
import json
import time
import threading
import math
import random
from datetime import datetime

app = Flask(__name__)

class SmartMeterBackend:
    def __init__(self):
        self.is_running = False
        self.simulation_data = {
            'voltage': 230.0,
            'current': 5.0,
            'frequency': 50.0,
            'power_factor': 0.95,
            'phase_config': 'single',
            'mcu_config': {
                'family': 'STM32F4',
                'part_number': 'STM32F407VG',
                'architecture': 'ARM Cortex-M4'
            },
            'measurements': {
                'voltage_rms': 0,
                'current_rms': 0,
                'active_power': 0,
                'reactive_power': 0,
                'apparent_power': 0,
                'power_factor': 0,
                'frequency': 0,
                'energy': 0
            },
            'peripherals': {
                'gpio': [{'pin': i, 'state': random.choice([True, False]), 'direction': 'input'} for i in range(16)],
                'adc': [{'channel': i, 'voltage': random.uniform(0, 3.3)} for i in range(8)],
                'timers': [{'timer': i, 'enabled': random.choice([True, False]), 'period': 1000} for i in range(4)]
            },
            'tamper_events': [],
            'protocol_log': [],
            'uart_log': []
        }
        self.firmware_loaded = False
        
        # Start background simulation thread
        self.simulation_thread = threading.Thread(target=self.simulation_loop, daemon=True)
        self.simulation_thread.start()

    def simulation_loop(self):
        while True:
            if self.is_running:
                self.update_measurements()
                self.update_peripherals()
            time.sleep(0.1)

    def update_measurements(self):
        voltage = self.simulation_data['voltage']
        current = self.simulation_data['current']
        power_factor = self.simulation_data['power_factor']
        
        # Add some noise for realistic simulation
        voltage_noise = random.uniform(-0.1, 0.1)
        current_noise = random.uniform(-0.01, 0.01)
        
        self.simulation_data['measurements'] = {
            'voltage_rms': voltage + voltage_noise,
            'current_rms': current + current_noise,
            'active_power': voltage * current * power_factor,
            'reactive_power': voltage * current * math.sin(math.acos(power_factor)),
            'apparent_power': voltage * current,
            'power_factor': power_factor,
            'frequency': self.simulation_data['frequency'] + random.uniform(-0.01, 0.01),
            'energy': self.simulation_data['measurements']['energy'] + (voltage * current * power_factor * 0.0001 / 3600)
        }

    def update_peripherals(self):
        # Update GPIO states randomly
        for gpio in self.simulation_data['peripherals']['gpio']:
            if random.random() < 0.1:  # 10% chance to change state
                gpio['state'] = not gpio['state']
        
        # Update ADC values with some variation
        for adc in self.simulation_data['peripherals']['adc']:
            adc['voltage'] = max(0, min(3.3, adc['voltage'] + random.uniform(-0.05, 0.05)))

    def load_firmware(self, filename, file_data):
        # Simulate firmware loading
        valid_extensions = ['.hex', '.bin']
        if not any(filename.lower().endswith(ext) for ext in valid_extensions):
            return False, "Invalid file format"
        
        # Simulate loading process
        time.sleep(0.5)
        self.firmware_loaded = True
        return True, "Firmware loaded successfully"

    def start_simulation(self):
        if not self.firmware_loaded:
            return False, "No firmware loaded"
        
        self.is_running = True
        return True, "Simulation started"

    def stop_simulation(self):
        self.is_running = False
        return True, "Simulation stopped"

    def reset_simulation(self):
        self.stop_simulation()
        self.simulation_data['measurements'] = {
            'voltage_rms': 0,
            'current_rms': 0,
            'active_power': 0,
            'reactive_power': 0,
            'apparent_power': 0,
            'power_factor': 0,
            'frequency': 0,
            'energy': 0
        }
        return True, "Simulation reset"

    def inject_tamper_event(self, tamper_type):
        event = {
            'type': tamper_type,
            'timestamp': datetime.now().isoformat(),
            'active': True
        }
        self.simulation_data['tamper_events'].append(event)
        return True, f"Tamper event injected: {tamper_type}"

    def process_protocol_command(self, protocol, command):
        timestamp = datetime.now().isoformat()
        
        # Log the command
        self.simulation_data['protocol_log'].append({
            'timestamp': timestamp,
            'direction': 'TX',
            'protocol': protocol,
            'data': command
        })
        
        # Generate response based on protocol
        response = self.generate_protocol_response(protocol, command)
        
        # Log the response
        self.simulation_data['protocol_log'].append({
            'timestamp': timestamp,
            'direction': 'RX',
            'protocol': protocol,
            'data': response
        })
        
        return response

    def generate_protocol_response(self, protocol, command):
        if protocol == 'dlms':
            if command.startswith('GET'):
                obis_codes = {
                    '1.0.1.8.0.255': f"{self.simulation_data['measurements']['energy']:.3f}*kWh",
                    '1.0.32.7.0.255': f"{self.simulation_data['measurements']['voltage_rms']:.2f}*V",
                    '1.0.31.7.0.255': f"{self.simulation_data['measurements']['current_rms']:.3f}*A",
                    '1.0.14.7.0.255': f"{self.simulation_data['measurements']['frequency']:.2f}*Hz"
                }
                parts = command.split()
                if len(parts) > 1 and parts[1] in obis_codes:
                    return f"DLMS Response: {parts[1]} = {obis_codes[parts[1]]}"
                return "ERROR: OBIS code not found"
            elif command.startswith('SET'):
                return "OK: Value set"
            return "ERROR: Invalid DLMS command"
        
        elif protocol == 'modbus-rtu':
            return "01 03 02 0C 35 A1"
        
        elif protocol == 'modbus-tcp':
            return "00 01 00 00 00 05 01 03 02 0C 35"
        
        elif protocol == 'iec62056':
            if command == '/?!':
                return "/SMT5\\2@1234567890"
            elif command.startswith('R'):
                return "1.8.0(12345.678*kWh)\r\n2.8.0(0.000*kWh)\r\n!"
            return "OK"
        
        return "OK"

    def send_uart_command(self, command):
        timestamp = datetime.now().isoformat()
        
        # Log TX
        self.simulation_data['uart_log'].append({
            'timestamp': timestamp,
            'direction': 'TX',
            'data': command
        })
        
        # Generate echo response
        response = f"ECHO: {command}"
        
        # Log RX
        self.simulation_data['uart_log'].append({
            'timestamp': timestamp,
            'direction': 'RX',
            'data': response
        })
        
        return response

# Global backend instance
backend = SmartMeterBackend()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/static/<path:filename>')
def static_files(filename):
    return send_from_directory('static', filename)

@app.route('/api/status')
def get_status():
    return jsonify({
        'is_running': backend.is_running,
        'firmware_loaded': backend.firmware_loaded,
        'measurements': backend.simulation_data['measurements'],
        'peripherals': backend.simulation_data['peripherals']
    })

@app.route('/api/firmware/load', methods=['POST'])
def load_firmware():
    if 'file' not in request.files:
        return jsonify({'success': False, 'message': 'No file provided'})
    
    file = request.files['file']
    if file.filename == '':
        return jsonify({'success': False, 'message': 'No file selected'})
    
    success, message = backend.load_firmware(file.filename, file.read())
    return jsonify({'success': success, 'message': message})

@app.route('/api/simulation/start', methods=['POST'])
def start_simulation():
    success, message = backend.start_simulation()
    return jsonify({'success': success, 'message': message})

@app.route('/api/simulation/stop', methods=['POST'])
def stop_simulation():
    success, message = backend.stop_simulation()
    return jsonify({'success': success, 'message': message})

@app.route('/api/simulation/reset', methods=['POST'])
def reset_simulation():
    success, message = backend.reset_simulation()
    return jsonify({'success': success, 'message': message})

@app.route('/api/configuration/update', methods=['POST'])
def update_configuration():
    data = request.json
    
    if 'voltage' in data:
        backend.simulation_data['voltage'] = float(data['voltage'])
    if 'current' in data:
        backend.simulation_data['current'] = float(data['current'])
    if 'frequency' in data:
        backend.simulation_data['frequency'] = float(data['frequency'])
    if 'power_factor' in data:
        backend.simulation_data['power_factor'] = float(data['power_factor'])
    if 'phase_config' in data:
        backend.simulation_data['phase_config'] = data['phase_config']
    
    return jsonify({'success': True, 'message': 'Configuration updated'})

@app.route('/api/tamper/inject', methods=['POST'])
def inject_tamper():
    data = request.json
    tamper_type = data.get('type', 'magnet')
    success, message = backend.inject_tamper_event(tamper_type)
    return jsonify({'success': success, 'message': message})

@app.route('/api/protocol/send', methods=['POST'])
def send_protocol_command():
    data = request.json
    protocol = data.get('protocol', 'dlms')
    command = data.get('command', '')
    
    response = backend.process_protocol_command(protocol, command)
    return jsonify({'success': True, 'response': response})

@app.route('/api/uart/send', methods=['POST'])
def send_uart_command():
    data = request.json
    command = data.get('command', '')
    
    response = backend.send_uart_command(command)
    return jsonify({'success': True, 'response': response})

@app.route('/api/logs/protocol')
def get_protocol_logs():
    return jsonify(backend.simulation_data['protocol_log'])

@app.route('/api/logs/uart')
def get_uart_logs():
    return jsonify(backend.simulation_data['uart_log'])

@app.route('/api/waveform')
def get_waveform_data():
    # Generate waveform data for charts
    time_now = time.time()
    frequency = backend.simulation_data['frequency']
    voltage_amplitude = backend.simulation_data['voltage']
    current_amplitude = backend.simulation_data['current']
    power_factor = backend.simulation_data['power_factor']
    
    voltage_data = []
    current_data = []
    
    for i in range(100):
        t = (time_now + i * 0.001) * 2 * math.pi * frequency
        voltage_data.append(voltage_amplitude * math.sin(t))
        current_data.append(current_amplitude * math.sin(t - math.acos(power_factor)))
    
    return jsonify({
        'voltage': voltage_data,
        'current': current_data,
        'time_labels': list(range(100))
    })

@app.route('/api/export/csv')
def export_csv():
    # Generate CSV data for export
    import io
    import csv
    from flask import make_response
    
    output = io.StringIO()
    writer = csv.writer(output)
    
    # Write headers
    writer.writerow(['Timestamp', 'Parameter', 'Value', 'Unit'])
    
    # Write measurement data
    measurements = backend.simulation_data['measurements']
    timestamp = datetime.now().isoformat()
    
    for param, value in measurements.items():
        unit = {
            'voltage_rms': 'V',
            'current_rms': 'A',
            'active_power': 'W',
            'reactive_power': 'VAR',
            'apparent_power': 'VA',
            'power_factor': '',
            'frequency': 'Hz',
            'energy': 'kWh'
        }.get(param, '')
        
        writer.writerow([timestamp, param, value, unit])
    
    response = make_response(output.getvalue())
    response.headers["Content-Disposition"] = f"attachment; filename=simulation_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
    response.headers["Content-type"] = "text/csv"
    
    return response

if __name__ == '__main__':
    # Create required directories
    os.makedirs('templates', exist_ok=True)
    os.makedirs('static/css', exist_ok=True)
    os.makedirs('static/js', exist_ok=True)
       # python
    if __name__ == '__main__':
        import os
        import argparse
        import sys
    
        parser = argparse.ArgumentParser()
        parser.add_argument("--port", type=int, help="Port to run the server on")
        parser.add_argument("--host", default=os.environ.get("HOST", "0.0.0.0"))
        args = parser.parse_args()
    
        # Create required directories
        os.makedirs('templates', exist_ok=True)
        os.makedirs('static/css', exist_ok=True)
        os.makedirs('static/js', exist_ok=True)
    
        port = args.port or int(os.environ.get("PORT", 5000))
        debug = bool(int(os.environ.get("FLASK_DEBUG", "1")))
    
        # When Flask debug is on, the reloader spawns a child process.
        # Only print startup messages from the reloader child to avoid duplicates.
        is_reloader_child = os.environ.get("WERKZEUG_RUN_MAIN") == "true" or not debug
        if is_reloader_child:
            print("Starting Smart Meter Firmware Simulator...")
            print(f"Access the simulator at: http://{args.host}:{port}")
    
        try:
            app.run(host=args.host, port=port, debug=debug, threaded=True)
        except OSError as e:
            err_no = getattr(e, "errno", None)
            if err_no in (48, 98):  # macOS/Linux "Address already in use"
                print(f"Error: Port {port} is already in use.")
                print("Options:")
                print(f"  - Run on a different port: python3 app.py --port 5001")
                print(f"  - Or kill the process using the port (macOS):")
                print(f"      lsof -nP -iTCP:{port} -sTCP:LISTEN")
                print("      kill <PID>")
                print("  - Or disable conflicting services (e.g. AirPlay Receiver in System Settings -> General -> Sharing).")
                sys.exit(1)
            raise 
    print("Starting Smart Meter Firmware Simulator...")
    print("Access the simulator at: http://localhost:5000")
    app.run(host='0.0.0.0', port=5000, debug=True, threaded=True)
