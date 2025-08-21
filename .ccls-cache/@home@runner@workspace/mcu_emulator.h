
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

struct MCUConfig {
    std::string family;
    std::string partNumber;
    std::string architecture;
    uint32_t flashSize;
    uint32_t ramSize;
    uint32_t clockFrequency;
    int adcChannels;
    int gpioCount;
    int timerCount;
};

struct GPIOPin {
    int pin;
    bool isOutput;
    bool state;
    std::string function;
};

struct ADCChannel {
    int channel;
    double voltage;
    uint16_t digitalValue;
    bool enabled;
};

struct TimerChannel {
    int timer;
    uint32_t period;
    uint32_t counter;
    bool enabled;
    bool pwmMode;
    double dutyCycle;
};

class MCUEmulator
{
public:
    MCUEmulator();
    ~MCUEmulator();

    bool loadFirmware(const std::string& firmwareFile);
    void configure(const std::string& family, const std::string& partNumber, const std::string& architecture);
    void reset();
    void update(double deltaTime);
    
    // Memory access
    uint8_t readByte(uint32_t address);
    void writeByte(uint32_t address, uint8_t value);
    uint32_t readWord(uint32_t address);
    void writeWord(uint32_t address, uint32_t value);
    
    // Peripheral access
    void setADCValue(int channel, double voltage);
    double getADCValue(int channel);
    void setGPIOState(int pin, bool state);
    bool getGPIOState(int pin);
    void configureGPIO(int pin, bool isOutput);
    
    // Timer/PWM
    void configureTimer(int timer, uint32_t period);
    void setTimerPWM(int timer, double dutyCycle);
    
    // UART/Communication
    void sendUARTData(const std::string& data);
    std::string receiveUARTData();
    
    // Status
    bool isRunning() const { return m_running; }
    uint32_t getProgramCounter() const { return m_programCounter; }
    const MCUConfig& getConfig() const { return m_config; }
    
    // Getters for UI
    const std::vector<GPIOPin>& getGPIOPins() const { return m_gpioPins; }
    const std::vector<ADCChannel>& getADCChannels() const { return m_adcChannels; }
    const std::vector<TimerChannel>& getTimers() const { return m_timers; }

private:
    void initializeMCU();
    void executeFirmware(double deltaTime);
    void updatePeripherals(double deltaTime);
    void processInterrupts();
    
    bool loadHexFile(const std::string& filename);
    bool loadBinFile(const std::string& filename);
    
    MCUConfig m_config;
    bool m_running;
    uint32_t m_programCounter;
    
    // Memory
    std::vector<uint8_t> m_flash;
    std::vector<uint8_t> m_ram;
    std::vector<uint8_t> m_eeprom;
    
    // Peripherals
    std::vector<GPIOPin> m_gpioPins;
    std::vector<ADCChannel> m_adcChannels;
    std::vector<TimerChannel> m_timers;
    
    // Communication
    std::string m_uartTxBuffer;
    std::string m_uartRxBuffer;
    
    // Simulation state
    double m_cycleTime;
    uint64_t m_totalCycles;
    
    // Interrupt system
    std::vector<bool> m_pendingInterrupts;
    std::map<int, std::function<void()>> m_interruptHandlers;
};
