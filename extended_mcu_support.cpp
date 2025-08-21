
#include "extended_mcu_support.h"
#include <algorithm>

MCUDatabase& MCUDatabase::getInstance()
{
    static MCUDatabase instance;
    return instance;
}

MCUDatabase::MCUDatabase()
{
    initializeDatabase();
}

void MCUDatabase::initializeDatabase()
{
    addSTM32MCUs();
    addNXPMCUs();
    addTIMCUs();
    addRenesasMCUs();
    addESPMCUs();
    addArduinoMCUs();
    addOtherMCUs();
    
    // Build part number lookup map
    for (const auto& mcu : m_mcuDatabase) {
        m_partNumberMap[mcu.partNumber] = mcu;
    }
}

void MCUDatabase::addSTM32MCUs()
{
    // STM32F4 Series
    m_mcuDatabase.push_back({
        "STM32F407VG", MCUFamily::STM32F4, MCUArchitecture::ARM_CORTEX_M4,
        1024, 192, 0, 168, 82, 16, 2, 14, 6, 3, 3, false, true, false, false, false,
        {"LQFP100", "BGA176"}, {{"FPU", "yes"}, {"DSP", "yes"}}
    });
    
    m_mcuDatabase.push_back({
        "STM32F429ZI", MCUFamily::STM32F4, MCUArchitecture::ARM_CORTEX_M4,
        2048, 256, 0, 180, 114, 24, 2, 17, 6, 3, 3, true, true, true, false, false,
        {"LQFP144", "BGA176"}, {{"FPU", "yes"}, {"LCD", "yes"}}
    });
    
    // STM32G0 Series
    m_mcuDatabase.push_back({
        "STM32G071RB", MCUFamily::STM32G0, MCUArchitecture::ARM_CORTEX_M0_PLUS,
        128, 36, 0, 64, 60, 12, 1, 11, 4, 2, 2, true, false, false, false, false,
        {"LQFP64", "QFN64"}, {{"LowPower", "yes"}}
    });
    
    // STM32H7 Series
    m_mcuDatabase.push_back({
        "STM32H743VI", MCUFamily::STM32H7, MCUArchitecture::ARM_CORTEX_M7,
        2048, 1024, 0, 480, 114, 20, 2, 17, 6, 4, 3, true, true, true, false, false,
        {"LQFP100", "BGA176"}, {{"FPU", "yes"}, {"DSP", "yes"}, {"Cache", "yes"}}
    });
}

void MCUDatabase::addESPMCUs()
{
    // ESP32 Series
    m_mcuDatabase.push_back({
        "ESP32-WROOM-32", MCUFamily::ESP32_WROOM, MCUArchitecture::ARM_CORTEX_M0,
        4096, 520, 0, 240, 34, 18, 2, 4, 10, 3, 2, false, true, true, true, true,
        {"Module"}, {{"WiFi", "802.11 b/g/n"}, {"Bluetooth", "4.2"}}
    });
    
    m_mcuDatabase.push_back({
        "ESP8266-12E", MCUFamily::ESP8266_WEMOS, MCUArchitecture::ARM_CORTEX_M0,
        4096, 80, 0, 80, 17, 1, 0, 1, 2, 1, 1, false, false, false, true, false,
        {"Module"}, {{"WiFi", "802.11 b/g/n"}}
    });
}

void MCUDatabase::addArduinoMCUs()
{
    // Arduino Uno (ATmega328P)
    m_mcuDatabase.push_back({
        "ATmega328P", MCUFamily::ARDUINO_UNO, MCUArchitecture::ATMEL_AVR,
        32, 2, 1, 16, 23, 8, 0, 3, 1, 1, 1, false, false, false, false, false,
        {"PDIP28", "TQFP32"}, {{"Arduino", "Uno"}}
    });
    
    // Arduino Mega (ATmega2560)
    m_mcuDatabase.push_back({
        "ATmega2560", MCUFamily::ARDUINO_MEGA, MCUArchitecture::ATMEL_AVR,
        256, 8, 4, 16, 86, 16, 0, 6, 4, 1, 1, false, false, false, false, false,
        {"TQFP100"}, {{"Arduino", "Mega"}}
    });
    
    // Raspberry Pi Pico (RP2040)
    m_mcuDatabase.push_back({
        "RP2040", MCUFamily::RASPBERRY_PI_PICO, MCUArchitecture::ARM_CORTEX_M0_PLUS,
        0, 264, 0, 133, 30, 4, 0, 8, 2, 2, 2, true, false, false, false, false,
        {"QFN56"}, {{"PIO", "yes"}, {"Dual Core", "yes"}}
    });
}

void MCUDatabase::addNXPMCUs()
{
    // LPC1768
    m_mcuDatabase.push_back({
        "LPC1768", MCUFamily::NXP_LPC17XX, MCUArchitecture::ARM_CORTEX_M3,
        512, 64, 0, 100, 70, 8, 1, 4, 4, 3, 2, true, true, true, false, false,
        {"LQFP100"}, {{"Ethernet", "yes"}}
    });
}

void MCUDatabase::addTIMCUs()
{
    // MSP430F5529
    m_mcuDatabase.push_back({
        "MSP430F5529", MCUFamily::TI_MSP430F, MCUArchitecture::TI_MSP430,
        128, 8, 0, 25, 63, 12, 2, 4, 4, 4, 2, true, false, false, false, false,
        {"LQFP80"}, {{"ULP", "yes"}}
    });
}

void MCUDatabase::addRenesasMCUs()
{
    // RL78/G13
    m_mcuDatabase.push_back({
        "RL78/G13", MCUFamily::RENESAS_RL78G1X, MCUArchitecture::RENESAS_RL78,
        64, 4, 4, 32, 31, 8, 1, 8, 2, 1, 1, false, false, false, false, false,
        {"LQFP48"}, {{"LowPower", "yes"}}
    });
}

void MCUDatabase::addOtherMCUs()
{
    // RISC-V example
    m_mcuDatabase.push_back({
        "FE310-G002", MCUFamily::NXP_LPC11XX, MCUArchitecture::RISC_V,
        0, 16, 0, 320, 19, 0, 0, 3, 2, 1, 1, false, false, false, false, false,
        {"QFN48"}, {{"RISC-V", "RV32IMAC"}}
    });
}

std::vector<MCUVariant> MCUDatabase::getMCUsByFamily(MCUFamily family)
{
    std::vector<MCUVariant> result;
    for (const auto& mcu : m_mcuDatabase) {
        if (mcu.family == family) {
            result.push_back(mcu);
        }
    }
    return result;
}

MCUVariant MCUDatabase::getMCUByPartNumber(const std::string& partNumber)
{
    auto it = m_partNumberMap.find(partNumber);
    if (it != m_partNumberMap.end()) {
        return it->second;
    }
    return {}; // Return empty variant if not found
}

bool MCUDatabase::isValidMCU(const std::string& partNumber)
{
    return m_partNumberMap.find(partNumber) != m_partNumberMap.end();
}

std::string MCUDatabase::getFamilyName(MCUFamily family)
{
    switch (family) {
        case MCUFamily::STM32F4: return "STM32F4";
        case MCUFamily::STM32G0: return "STM32G0";
        case MCUFamily::STM32H7: return "STM32H7";
        case MCUFamily::ESP32_WROOM: return "ESP32";
        case MCUFamily::ESP8266_WEMOS: return "ESP8266";
        case MCUFamily::ARDUINO_UNO: return "Arduino Uno";
        case MCUFamily::ARDUINO_MEGA: return "Arduino Mega";
        case MCUFamily::RASPBERRY_PI_PICO: return "Raspberry Pi Pico";
        default: return "Unknown";
    }
}

std::string MCUDatabase::getArchitectureName(MCUArchitecture arch)
{
    switch (arch) {
        case MCUArchitecture::ARM_CORTEX_M0: return "ARM Cortex-M0";
        case MCUArchitecture::ARM_CORTEX_M0_PLUS: return "ARM Cortex-M0+";
        case MCUArchitecture::ARM_CORTEX_M3: return "ARM Cortex-M3";
        case MCUArchitecture::ARM_CORTEX_M4: return "ARM Cortex-M4";
        case MCUArchitecture::ARM_CORTEX_M7: return "ARM Cortex-M7";
        case MCUArchitecture::RISC_V: return "RISC-V";
        case MCUArchitecture::ATMEL_AVR: return "AVR";
        case MCUArchitecture::TI_MSP430: return "MSP430";
        case MCUArchitecture::RENESAS_RL78: return "RL78";
        default: return "Unknown";
    }
}

// ExtendedMCUEmulator implementation
ExtendedMCUEmulator::ExtendedMCUEmulator()
    : m_running(false)
    , m_loaded(false)
    , m_pc(0)
    , m_sp(0)
    , m_cycleCount(0)
    , m_clockFrequency(1000000)
{
}

ExtendedMCUEmulator::~ExtendedMCUEmulator() = default;

bool ExtendedMCUEmulator::loadMCU(const std::string& partNumber)
{
    MCUDatabase& db = MCUDatabase::getInstance();
    if (!db.isValidMCU(partNumber)) {
        return false;
    }
    
    m_currentMCU = db.getMCUByPartNumber(partNumber);
    
    // Initialize memory
    m_flash.resize(m_currentMCU.flashSize * 1024);
    m_ram.resize(m_currentMCU.ramSize * 1024);
    if (m_currentMCU.eepromSize > 0) {
        m_eeprom.resize(m_currentMCU.eepromSize * 1024);
    }
    
    initializePeripherals();
    m_loaded = true;
    return true;
}

bool ExtendedMCUEmulator::loadFirmware(const std::string& firmwareFile)
{
    if (!m_loaded) return false;
    
    // Simplified firmware loading - would need proper hex/bin parsing
    return true;
}

void ExtendedMCUEmulator::reset()
{
    m_pc = 0;
    m_sp = m_ram.size() - 4; // Top of RAM
    m_cycleCount = 0;
    m_running = false;
    
    // Reset peripherals
    m_peripheralRegisters.clear();
    initializePeripherals();
}

void ExtendedMCUEmulator::initializePeripherals()
{
    // Initialize basic peripheral registers based on MCU type
    for (int i = 0; i < m_currentMCU.gpioCount; i++) {
        m_peripheralRegisters[std::string("GPIO") + std::to_string(i) + "_DIR"] = 0;
        m_peripheralRegisters[std::string("GPIO") + std::to_string(i) + "_OUT"] = 0;
        m_peripheralRegisters[std::string("GPIO") + std::to_string(i) + "_IN"] = 0;
    }
    
    for (int i = 0; i < m_currentMCU.adcChannels; i++) {
        m_peripheralRegisters[std::string("ADC") + std::to_string(i) + "_VAL"] = 0;
    }
}

void ExtendedMCUEmulator::run()
{
    m_running = true;
}

void ExtendedMCUEmulator::stop()
{
    m_running = false;
}

void ExtendedMCUEmulator::step()
{
    if (m_loaded && m_pc < m_flash.size()) {
        executeInstruction();
        updatePeripherals();
        m_cycleCount++;
    }
}

void ExtendedMCUEmulator::executeInstruction()
{
    // Simplified instruction execution - would need proper decoder
    m_pc += 2; // Assume 16-bit instructions for simplicity
}

void ExtendedMCUEmulator::updatePeripherals()
{
    // Update peripheral state based on configuration
}
