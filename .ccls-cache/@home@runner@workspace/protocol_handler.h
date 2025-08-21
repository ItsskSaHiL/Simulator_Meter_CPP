
#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

class ProtocolHandler
{
public:
    ProtocolHandler();
    ~ProtocolHandler();

    void update(double deltaTime);
    std::string processCommand(const std::string& protocol, const std::string& command);
    
    // Protocol support
    void enableProtocol(const std::string& protocol);
    void disableProtocol(const std::string& protocol);
    std::vector<std::string> getSupportedProtocols() const;
    
    // DLMS/COSEM
    std::string processDLMSCommand(const std::string& command);
    
    // Modbus
    std::string processModbusCommand(const std::string& command);
    
    // IEC 62056
    std::string processIEC62056Command(const std::string& command);
    
    // Custom protocol support
    void registerCustomProtocol(const std::string& name, 
                               std::function<std::string(const std::string&)> handler);

private:
    std::map<std::string, bool> m_enabledProtocols;
    std::map<std::string, std::function<std::string(const std::string&)>> m_customProtocols;
    
    // Protocol-specific data
    std::map<std::string, std::string> m_meterData;
    
    void initializeProtocols();
    void initializeMeterData();
    
    // Helper functions
    std::string formatDLMSResponse(const std::string& obis, const std::string& value);
    std::string formatModbusResponse(uint16_t address, uint16_t value);
    std::string parseModbusCommand(const std::string& command);
    std::string parseHexString(const std::string& hex);
    std::string toHexString(const std::vector<uint8_t>& data);
};
