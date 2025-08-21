
# Smart Meter Firmware Simulator

A comprehensive software simulator for testing the firmware of electric smart meters. This simulator provides a virtual hardware environment that emulates microcontrollers, metering peripherals, and communication protocols commonly used in smart meter applications.

## Features

### Firmware Loading & Execution
- Support for .hex and .bin firmware files
- Multiple MCU family support (STM32F4, STM32F7, Renesas RL78, TI MSP430, etc.)
- Configurable target specifications (part number, package, architecture)
- Virtual flash/ROM/RAM memory emulation
- Basic firmware execution simulation

### Hardware & Peripheral Simulation
- **MCU Peripherals**: UART, I2C, SPI, CAN with interactive terminals
- **Analog**: ADC/DAC with configurable voltage/current waveforms
- **Digital**: Timers, PWM, GPIO with user control
- **Communication**: Virtual connectors for testing protocols

### Metering Features
- Single-phase and three-phase AC input simulation
- Configurable voltage, current, frequency, and power factor
- Real-time calculation of RMS values, power, and energy
- Waveform generation and display
- Harmonic injection and THD calculation
- Configurable tamper event simulation:
  - Magnet tamper
  - Reverse current flow
  - Neutral missing
  - Phase loss
  - Over/under voltage

### Communication Protocols
- **DLMS/COSEM**: Basic OBIS code support and association handling
- **Modbus RTU/TCP**: Register read/write operations
- **IEC 62056**: Optical interface simulation
- **Custom Protocol**: Extensible framework for proprietary protocols

### User Interface
- Real-time dashboard with measurements and graphs
- Interactive waveform display
- Protocol communication terminal
- GPIO and peripheral status displays
- Event logging and export capabilities
- Automated test case scripting support

### Configuration & Extensibility
- JSON-based configuration for hardware profiles
- Modular design for adding new MCUs and protocols
- Signal injection for testing (voltage dips, noise, tamper events)
- CSV/JSON export for analysis
- Cross-platform support (Linux/Windows)

## Building

### Prerequisites
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- Qt5 development libraries
- pkg-config

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential qt5-default qtbase5-dev pkg-config
```

### Build Commands
```bash
# Build the simulator
make

# Build with debug information
make debug

# Build and run
make run

# Clean build files
make clean
```

## Usage

### Basic Operation

1. **Load Firmware**:
   - Click "Browse" to select a .hex or .bin firmware file
   - Configure the target MCU family, part number, and architecture
   - Click "Load Firmware"

2. **Configure Metering**:
   - Set phase configuration (single or three-phase)
   - Configure voltage, current, frequency, and power factor
   - Optionally inject harmonics or noise

3. **Start Simulation**:
   - Click "Start" to begin firmware execution
   - Monitor real-time measurements and waveforms
   - Observe peripheral status and communication

4. **Test Protocols**:
   - Select a communication protocol (DLMS, Modbus, IEC 62056)
   - Send commands and observe responses
   - Monitor protocol logs

5. **Inject Events**:
   - Select tamper event type
   - Click "Inject Tamper Event" to test firmware response
   - Monitor system logs for event detection

### Example Commands

#### DLMS/COSEM
```
GET 1.0.1.8.0.255    # Read active energy import
GET 1.0.32.7.0.255   # Read voltage L1
SET 0.0.96.1.0.255 SMT123456789  # Set meter serial number
```

#### Modbus
```
01 03 00 00 00 01    # Read holding register 40001
01 06 00 64 03 E8    # Write 1000 to register 40101
```

#### IEC 62056
```
/?!                  # Identification request
R1                   # Read data
```

## Configuration

The simulator uses `config.json` for default settings:

- **MCU Profiles**: Define hardware specifications for different microcontrollers
- **Metering Parameters**: Set default electrical parameters
- **Protocol Settings**: Configure communication protocols
- **Tamper Events**: Define detection thresholds
- **Logging**: Configure log levels and file rotation

## Architecture

The simulator is built with a modular architecture:

- **SimulatorCore**: Main simulation engine and timing control
- **MCUEmulator**: Microcontroller hardware emulation
- **MeteringEngine**: Electrical measurement simulation
- **ProtocolHandler**: Communication protocol processing
- **GUI**: Qt-based user interface

## Extending the Simulator

### Adding New MCUs
1. Add MCU profile to `config.json`
2. Implement specific peripheral behaviors in `MCUEmulator`
3. Update GUI MCU selection lists

### Adding New Protocols
1. Implement protocol handler in `ProtocolHandler`
2. Add protocol to configuration
3. Update GUI protocol selection

### Adding Custom Tamper Events
1. Define event in `config.json`
2. Implement detection logic in `MeteringEngine`
3. Add UI controls for event injection

## Testing

The simulator includes several test scenarios:

- **Basic Metering**: Verify voltage, current, and power measurements
- **Protocol Communication**: Test command/response handling
- **Tamper Detection**: Validate firmware response to tamper events
- **Signal Quality**: Test with harmonics, noise, and disturbances
- **Load Profile**: Simulate varying load conditions

## Troubleshooting

### Common Issues

1. **Firmware Load Failure**:
   - Verify file format (.hex or .bin)
   - Check file permissions
   - Ensure MCU configuration matches firmware

2. **Protocol Communication Issues**:
   - Verify protocol is enabled
   - Check command syntax
   - Monitor protocol logs for errors

3. **Build Issues**:
   - Install Qt5 development packages
   - Update pkg-config database
   - Check compiler version compatibility

### Debug Mode
Build with debug information for detailed logging:
```bash
make debug
./smart_meter_simulator --debug
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Implement changes with tests
4. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Qt Framework for the user interface
- Various smart meter standards organizations (IEC, DLMS, Modbus)
- Open source embedded simulation projects

## Contact

For questions, issues, or contributions, please open an issue on the project repository.
