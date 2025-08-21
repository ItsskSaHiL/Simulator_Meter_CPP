
#include "protocol_handler.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

ProtocolHandler::ProtocolHandler()
{
    initializeProtocols();
    initializeMeterData();
}

ProtocolHandler::~ProtocolHandler() = default;

void ProtocolHandler::initializeProtocols()
{
    m_enabledProtocols["DLMS/COSEM"] = true;
    m_enabledProtocols["Modbus RTU"] = true;
    m_enabledProtocols["Modbus TCP"] = true;
    m_enabledProtocols["IEC 62056"] = true;
    m_enabledProtocols["Custom"] = false;
}

void ProtocolHandler::initializeMeterData()
{
    // OBIS codes for DLMS/COSEM
    m_meterData["1.0.1.8.0.255"] = "12345.678"; // Active energy import
    m_meterData["1.0.2.8.0.255"] = "0.000";     // Active energy export
    m_meterData["1.0.32.7.0.255"] = "230.5";    // Voltage L1
    m_meterData["1.0.52.7.0.255"] = "230.2";    // Voltage L2
    m_meterData["1.0.72.7.0.255"] = "230.8";    // Voltage L3
    m_meterData["1.0.31.7.0.255"] = "5.234";    // Current L1
    m_meterData["1.0.51.7.0.255"] = "5.156";    // Current L2
    m_meterData["1.0.71.7.0.255"] = "5.298";    // Current L3
    m_meterData["1.0.14.7.0.255"] = "50.02";    // Frequency
    m_meterData["1.0.13.7.0.255"] = "0.95";     // Power factor
    m_meterData["0.0.96.1.0.255"] = "SMT001234567890"; // Meter serial number
    m_meterData["1.0.0.2.0.255"] = "v1.2.3";    // Firmware version
    
    // Modbus register map
    m_meterData["modbus_40001"] = "2305";   // Voltage L1 (V * 10)
    m_meterData["modbus_40002"] = "2302";   // Voltage L2 (V * 10)
    m_meterData["modbus_40003"] = "2308";   // Voltage L3 (V * 10)
    m_meterData["modbus_40004"] = "523";    // Current L1 (A * 100)
    m_meterData["modbus_40005"] = "516";    // Current L2 (A * 100)
    m_meterData["modbus_40006"] = "530";    // Current L3 (A * 100)
    m_meterData["modbus_40007"] = "5002";   // Frequency (Hz * 100)
    m_meterData["modbus_40008"] = "95";     // Power factor (* 100)
}

void ProtocolHandler::update(double deltaTime)
{
    // Update any time-sensitive protocol operations
    // Handle periodic tasks, timeouts, etc.
}

std::string ProtocolHandler::processCommand(const std::string& protocol, const std::string& command)
{
    if (!m_enabledProtocols[protocol]) {
        return "ERROR: Protocol not enabled";
    }
    
    if (protocol == "DLMS/COSEM") {
        return processDLMSCommand(command);
    } else if (protocol == "Modbus RTU" || protocol == "Modbus TCP") {
        return processModbusCommand(command);
    } else if (protocol == "IEC 62056") {
        return processIEC62056Command(command);
    } else if (protocol == "Custom") {
        auto it = m_customProtocols.find("default");
        if (it != m_customProtocols.end()) {
            return it->second(command);
        }
        return "ERROR: Custom protocol handler not registered";
    }
    
    return "ERROR: Unknown protocol";
}

std::string ProtocolHandler::processDLMSCommand(const std::string& command)
{
    // Simplified DLMS/COSEM command processing
    // In a real implementation, this would handle the full DLMS protocol stack
    
    // Check for GET request with OBIS code
    if (command.find("GET") == 0) {
        // Extract OBIS code (format: GET 1.0.1.8.0.255)
        std::istringstream iss(command);
        std::string cmd, obis;
        iss >> cmd >> obis;
        
        auto it = m_meterData.find(obis);
        if (it != m_meterData.end()) {
            return formatDLMSResponse(obis, it->second);
        } else {
            return "ERROR: OBIS code not found: " + obis;
        }
    }
    // Check for SET request
    else if (command.find("SET") == 0) {
        std::istringstream iss(command);
        std::string cmd, obis, value;
        iss >> cmd >> obis >> value;
        
        // Only allow setting certain values (not measurements)
        if (obis.find("96.1.0") != std::string::npos) { // Serial number
            m_meterData[obis] = value;
            return "OK: " + obis + " set to " + value;
        } else {
            return "ERROR: Cannot set read-only OBIS code: " + obis;
        }
    }
    // Association request
    else if (command.find("AARQ") == 0) {
        return "AARE: Association established";
    }
    // Release request
    else if (command.find("RLRQ") == 0) {
        return "RLRE: Association released";
    }
    
    return "ERROR: Invalid DLMS command: " + command;
}

std::string ProtocolHandler::processModbusCommand(const std::string& command)
{
    // Simplified Modbus command processing
    // Format: Function Code + Address + Data (in hex)
    
    try {
        // Parse hex command
        std::vector<uint8_t> data;
        std::string hex = command;
        
        // Remove spaces and convert to uppercase
        hex.erase(std::remove(hex.begin(), hex.end(), ' '), hex.end());
        std::transform(hex.begin(), hex.end(), hex.begin(), ::toupper);
        
        // Convert hex string to bytes
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::strtol(byteString.c_str(), nullptr, 16));
            data.push_back(byte);
        }
        
        if (data.size() < 6) {
            return "ERROR: Invalid Modbus command length";
        }
        
        uint8_t slaveId = data[0];
        uint8_t functionCode = data[1];
        uint16_t address = (data[2] << 8) | data[3];
        uint16_t quantity = (data[4] << 8) | data[5];
        
        // Function Code 03: Read Holding Registers
        if (functionCode == 0x03) {
            std::vector<uint8_t> response;
            response.push_back(slaveId);
            response.push_back(functionCode);
            response.push_back(quantity * 2); // Byte count
            
            for (uint16_t i = 0; i < quantity; i++) {
                uint16_t regAddress = address + i;
                std::string regKey = "modbus_" + std::to_string(40000 + regAddress);
                
                uint16_t regValue = 0;
                auto it = m_meterData.find(regKey);
                if (it != m_meterData.end()) {
                    regValue = static_cast<uint16_t>(std::stoi(it->second));
                }
                
                response.push_back((regValue >> 8) & 0xFF);
                response.push_back(regValue & 0xFF);
            }
            
            // Add CRC (simplified - just add dummy bytes)
            response.push_back(0x00);
            response.push_back(0x00);
            
            return toHexString(response);
        }
        // Function Code 06: Write Single Register
        else if (functionCode == 0x06) {
            if (data.size() < 8) {
                return "ERROR: Invalid write command length";
            }
            
            uint16_t value = (data[4] << 8) | data[5];
            std::string regKey = "modbus_" + std::to_string(40000 + address);
            
            // Only allow writing to certain registers
            if (address >= 100) { // Configuration registers
                m_meterData[regKey] = std::to_string(value);
                
                // Echo back the command for success
                return toHexString(data);
            } else {
                // Return exception response
                std::vector<uint8_t> response = {slaveId, static_cast<uint8_t>(functionCode | 0x80), 0x02, 0x00, 0x00};
                return toHexString(response);
            }
        }
        
        return "ERROR: Unsupported function code: " + std::to_string(functionCode);
        
    } catch (const std::exception& e) {
        return "ERROR: Failed to parse Modbus command: " + std::string(e.what());
    }
}

std::string ProtocolHandler::processIEC62056Command(const std::string& command)
{
    // Simplified IEC 62056-21 (optical interface) command processing
    
    // Identification request
    if (command == "/?!") {
        return "/SMT5\\2@1234567890\r\n";
    }
    // Programming mode request
    else if (command.find("#") == 0) {
        return "OK\r\n";
    }
    // Read data request
    else if (command.find("R1") == 0) {
        std::string response;
        response += "1.8.0(12345.678*kWh)\r\n";
        response += "2.8.0(0.000*kWh)\r\n";
        response += "32.7.0(230.5*V)\r\n";
        response += "31.7.0(5.234*A)\r\n";
        response += "14.7.0(50.02*Hz)\r\n";
        response += "!\r\n";
        return response;
    }
    // Break command
    else if (command.find("B0") == 0) {
        return "B0\r\n";
    }
    
    return "ERROR: Unknown IEC 62056 command: " + command;
}

void ProtocolHandler::enableProtocol(const std::string& protocol)
{
    m_enabledProtocols[protocol] = true;
}

void ProtocolHandler::disableProtocol(const std::string& protocol)
{
    m_enabledProtocols[protocol] = false;
}

std::vector<std::string> ProtocolHandler::getSupportedProtocols() const
{
    std::vector<std::string> protocols;
    for (const auto& protocol : m_enabledProtocols) {
        if (protocol.second) {
            protocols.push_back(protocol.first);
        }
    }
    return protocols;
}

void ProtocolHandler::registerCustomProtocol(const std::string& name, 
                                           std::function<std::string(const std::string&)> handler)
{
    m_customProtocols[name] = handler;
    m_enabledProtocols["Custom"] = true;
}

std::string ProtocolHandler::formatDLMSResponse(const std::string& obis, const std::string& value)
{
    return "DLMS Response: " + obis + " = " + value;
}

std::string ProtocolHandler::formatModbusResponse(uint16_t address, uint16_t value)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    oss << std::setw(4) << address << " = " << std::setw(4) << value;
    return oss.str();
}

std::string ProtocolHandler::toHexString(const std::vector<uint8_t>& data)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    
    for (size_t i = 0; i < data.size(); i++) {
        if (i > 0) oss << " ";
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    
    return oss.str();
}
