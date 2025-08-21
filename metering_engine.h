
#pragma once

#include <vector>
#include <string>
#include <map>
#include <chrono>

struct PhasorData {
    double magnitude;
    double phase;  // in degrees
    double real;
    double imag;
};

struct HarmonicData {
    double magnitude;
    double phase;
    double percentage; // as % of fundamental
};

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
    
    // Phasor data
    PhasorData voltagePhasor[3];   // Voltage phasors for each phase
    PhasorData currentPhasor[3];   // Current phasors for each phase
    
    // Harmonics up to 33rd order
    HarmonicData voltageHarmonics[33];  // 1st to 33rd harmonic
    HarmonicData currentHarmonics[33];  // 1st to 33rd harmonic
    
    // Additional power quality parameters
    double crest_factor_voltage;
    double crest_factor_current;
    double k_factor;               // K-factor for transformer derating
    double displacement_pf;        // Displacement power factor (fundamental)
    double distortion_pf;         // Distortion power factor
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
    void injectHarmonics(int harmonic, double magnitude, double phase = 0.0);
    void injectNoise(double amplitude);
    void injectInterharmonics(double frequency, double magnitude);
    
    // Harmonics and phasor analysis
    void calculateHarmonics();
    void calculatePhasors();
    std::vector<HarmonicData> getVoltageHarmonics() const;
    std::vector<HarmonicData> getCurrentHarmonics() const;
    std::vector<PhasorData> getVoltagePhasors() const;
    std::vector<PhasorData> getCurrentPhasors() const;
    
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
    
    // Harmonics with phase information
    std::map<int, std::pair<double, double>> m_harmonics; // harmonic number -> (magnitude, phase)
    std::map<double, double> m_interharmonics; // frequency -> magnitude
    double m_noiseAmplitude;
    
    // FFT and analysis
    void performFFT(const std::vector<double>& samples, std::vector<std::complex<double>>& fft_result);
    void calculateCrestFactor();
    void calculateKFactor();
    void calculatePowerFactorComponents();
};
