
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

enum class MCUArchitecture {
    ARM_CORTEX_M0,
    ARM_CORTEX_M0_PLUS,
    ARM_CORTEX_M3,
    ARM_CORTEX_M4,
    ARM_CORTEX_M7,
    ARM_CORTEX_M33,
    RENESAS_RL78,
    RENESAS_RX,
    ESPRESSIF_ESP32,
    ESPRESSIF_ESP8266,
    TI_MSP430,
    ATMEL_AVR,
    PIC16,
    PIC32,
    MIPS32,
    RISC_V
};

enum class MCUFamily {
    STM32F0,
    STM32F1,
    STM32F2,
    STM32F3,
    STM32F4,
    STM32F7,
    STM32G0,
    STM32G4,
    STM32H7,
    STM32L0,
    STM32L4,
    STM32L5,
    STM32U5,
    STM32WB,
    STM32WL,
    NXP_LPC11XX,
    NXP_LPC17XX,
    NXP_LPC43XX,
    NXP_KINETIS_K,
    NXP_KINETIS_L,
    TI_MSP430G,
    TI_MSP430F,
    RENESAS_RL78G1X,
    RENESAS_RL78G2X,
    RENESAS_RX,
    ESP32_WROOM,
    ESP32_S2,
    ESP32_S3,
    ESP32_C3,
    ESP8266_WEMOS,
    ARDUINO_UNO,
    ARDUINO_MEGA,
    RASPBERRY_PI_PICO,
    MAXIM_78XX,
    ANALOG_DEVICES_ADE,
    MICROCHIP_PIC32,
    SILABS_EFR32
};

struct MCUVariant {
    std::string partNumber;
    MCUFamily family;
    MCUArchitecture architecture;
    uint32_t flashSize;       // in KB
    uint32_t ramSize;         // in KB
    uint32_t eepromSize;      // in KB
    uint32_t maxClockFreq;    // in MHz
    int gpioCount;
    int adcChannels;
    int dacChannels;
    int timerCount;
    int uartCount;
    int spiCount;
    int i2cCount;
    bool hasUsb;
    bool hasCan;
    bool hasEthernet;
    bool hasWifi;
    bool hasBluetooth;
    std::vector<std::string> packages;
    std::map<std::string, std::string> features;
};

class MCUDatabase {
public:
    static MCUDatabase& getInstance();
    
    std::vector<MCUVariant> getMCUsByFamily(MCUFamily family);
    std::vector<MCUVariant> getMCUsByArchitecture(MCUArchitecture arch);
    MCUVariant getMCUByPartNumber(const std::string& partNumber);
    std::vector<std::string> getAllPartNumbers();
    std::vector<MCUFamily> getAllFamilies();
    std::vector<MCUArchitecture> getAllArchitectures();
    
    bool isValidMCU(const std::string& partNumber);
    std::string getFamilyName(MCUFamily family);
    std::string getArchitectureName(MCUArchitecture arch);

private:
    MCUDatabase();
    void initializeDatabase();
    void addSTM32MCUs();
    void addNXPMCUs();
    void addTIMCUs();
    void addRenesasMCUs();
    void addESPMCUs();
    void addArduinoMCUs();
    void addOtherMCUs();

    std::vector<MCUVariant> m_mcuDatabase;
    std::map<std::string, MCUVariant> m_partNumberMap;
};

class ExtendedMCUEmulator {
public:
    ExtendedMCUEmulator();
    ~ExtendedMCUEmulator();
    
    bool loadMCU(const std::string& partNumber);
    bool loadFirmware(const std::string& firmwareFile);
    void reset();
    void step();
    void run();
    void stop();
    
    // Debug interface
    void setBreakpoint(uint32_t address);
    void removeBreakpoint(uint32_t address);
    std::vector<uint32_t> getBreakpoints();
    bool isAtBreakpoint();
    
    // Register access
    uint32_t readRegister(const std::string& regName);
    void writeRegister(const std::string& regName, uint32_t value);
    std::map<std::string, uint32_t> getAllRegisters();
    
    // Memory access
    std::vector<uint8_t> readMemory(uint32_t address, size_t size);
    void writeMemory(uint32_t address, const std::vector<uint8_t>& data);
    
    // Peripheral simulation
    void simulateGPIO();
    void simulateADC();
    void simulateUART();
    void simulateSPI();
    void simulateI2C();
    void simulateTimers();
    
    // Status
    bool isRunning() const { return m_running; }
    uint32_t getProgramCounter() const { return m_pc; }
    uint32_t getStackPointer() const { return m_sp; }
    const MCUVariant& getCurrentMCU() const { return m_currentMCU; }

private:
    void initializePeripherals();
    void executeInstruction();
    void handleInterrupt();
    void updatePeripherals();

    MCUVariant m_currentMCU;
    bool m_running;
    bool m_loaded;
    
    // CPU state
    uint32_t m_pc;
    uint32_t m_sp;
    std::map<std::string, uint32_t> m_registers;
    std::vector<uint32_t> m_breakpoints;
    
    // Memory
    std::vector<uint8_t> m_flash;
    std::vector<uint8_t> m_ram;
    std::vector<uint8_t> m_eeprom;
    
    // Peripheral state
    std::map<std::string, uint32_t> m_peripheralRegisters;
    
    // Simulation state
    uint64_t m_cycleCount;
    double m_clockFrequency;
};
