
#pragma once

#include <vector>
#include <string>
#include <map>
#include <chrono>

struct MeteringMeasurements {
    double voltageRMS;
    double currentRMS;
    double activePower;
    double reactivePower;
    double apparentPower;
    double powerFactor;
    double frequency;
    double energy;
    double voltage[3];  // Phase voltages for 3-phase
    double current[3];  // Phase currents for 3-phase
    double thd_voltage;
    double thd_current;
};

struct TamperEvent {
    std::string type;
    std::chrono::system_clock::time_point timestamp;
    bool active;
    std::map<std::string, double> parameters;
};

class MeteringEngine
{
public:
    MeteringEngine();
    ~MeteringEngine();

    void configure(bool threePhase, double voltage, double current, double frequency, double powerFactor);
    void reset();
    void update(double deltaTime);
    
    // Measurements
    const MeteringMeasurements& getMeasurements() const { return m_measurements; }
    std::vector<double> getVoltageWaveform() const;
    std::vector<double> getCurrentWaveform() const;
    
    // Tamper events
    void injectTamperEvent(const std::string& type);
    void clearTamperEvent(const std::string& type);
    std::vector<TamperEvent> getActiveTamperEvents() const;
    
    // Configuration
    void setVoltage(double voltage) { m_configVoltage = voltage; }
    void setCurrent(double current) { m_configCurrent = current; }
    void setFrequency(double frequency) { m_configFrequency = frequency; }
    void setPowerFactor(double pf) { m_configPowerFactor = pf; }
    void setPhaseConfiguration(bool threePhase) { m_isThreePhase = threePhase; }
    
    // Signal injection
    void injectVoltageDip(double magnitude, double duration);
    void injectFrequencyVariation(double deviation, double duration);
    void injectHarmonics(int harmonic, double magnitude);
    void injectNoise(double amplitude);
    
    // Relay control
    void setRelayState(bool connected) { m_relayConnected = connected; }
    bool getRelayState() const { return m_relayConnected; }

private:
    void calculateMeasurements();
    void updateWaveforms(double deltaTime);
    void processTamperEvents();
    void generateSignals(double time);
    
    double calculateRMS(const std::vector<double>& samples);
    double calculateTHD(const std::vector<double>& samples);
    
    // Configuration
    bool m_isThreePhase;
    double m_configVoltage;
    double m_configCurrent;
    double m_configFrequency;
    double m_configPowerFactor;
    
    // Current measurements
    MeteringMeasurements m_measurements;
    
    // Waveform data
    std::vector<double> m_voltageWaveform;
    std::vector<double> m_currentWaveform;
    std::vector<std::vector<double>> m_voltageWaveforms3P;  // 3-phase voltages
    std::vector<std::vector<double>> m_currentWaveforms3P;  // 3-phase currents
    
    // Simulation state
    double m_simulationTime;
    double m_phaseAngle;
    static constexpr int SAMPLES_PER_CYCLE = 256;
    static constexpr double SAMPLE_RATE = 12800.0; // 256 samples * 50Hz
    
    // Tamper events
    std::map<std::string, TamperEvent> m_tamperEvents;
    
    // Signal injection
    struct SignalInjection {
        bool active;
        double startTime;
        double duration;
        double magnitude;
        std::string type;
    };
    std::vector<SignalInjection> m_injections;
    
    // Energy measurement
    double m_totalEnergy;
    double m_lastPowerSample;
    
    // Relay state
    bool m_relayConnected;
    
    // Harmonics
    std::map<int, double> m_harmonics; // harmonic number -> magnitude
    double m_noiseAmplitude;
};
