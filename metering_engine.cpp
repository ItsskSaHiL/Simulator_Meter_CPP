
#include "metering_engine.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>
#include <complex>
#include <numeric>

MeteringEngine::MeteringEngine()
    : m_isThreePhase(false)
    , m_configVoltage(230.0)
    , m_configCurrent(5.0)
    , m_configFrequency(50.0)
    , m_configPowerFactor(0.95)
    , m_simulationTime(0.0)
    , m_phaseAngle(0.0)
    , m_totalEnergy(0.0)
    , m_lastPowerSample(0.0)
    , m_relayConnected(true)
    , m_noiseAmplitude(0.0)
{
    reset();
}

MeteringEngine::~MeteringEngine() = default;

void MeteringEngine::configure(bool threePhase, double voltage, double current, double frequency, double powerFactor)
{
    m_isThreePhase = threePhase;
    m_configVoltage = voltage;
    m_configCurrent = current;
    m_configFrequency = frequency;
    m_configPowerFactor = powerFactor;
    
    // Initialize waveform arrays
    m_voltageWaveform.resize(SAMPLES_PER_CYCLE);
    m_currentWaveform.resize(SAMPLES_PER_CYCLE);
    
    if (m_isThreePhase) {
        m_voltageWaveforms3P.resize(3);
        m_currentWaveforms3P.resize(3);
        for (int i = 0; i < 3; i++) {
            m_voltageWaveforms3P[i].resize(SAMPLES_PER_CYCLE);
            m_currentWaveforms3P[i].resize(SAMPLES_PER_CYCLE);
        }
    }
}

void MeteringEngine::reset()
{
    m_simulationTime = 0.0;
    m_phaseAngle = 0.0;
    m_totalEnergy = 0.0;
    m_lastPowerSample = 0.0;
    m_relayConnected = true;
    
    // Clear measurements
    m_measurements = {};
    
    // Initialize phasor arrays
    for (int i = 0; i < 3; i++) {
        m_measurements.voltagePhasor[i] = {0.0, 0.0, 0.0, 0.0};
        m_measurements.currentPhasor[i] = {0.0, 0.0, 0.0, 0.0};
    }
    
    // Initialize harmonics arrays
    for (int i = 0; i < 33; i++) {
        m_measurements.voltageHarmonics[i] = {0.0, 0.0, 0.0};
        m_measurements.currentHarmonics[i] = {0.0, 0.0, 0.0};
    }
    
    // Clear tamper events
    m_tamperEvents.clear();
    
    // Clear signal injections
    m_injections.clear();
    
    // Clear harmonics
    m_harmonics.clear();
    m_interharmonics.clear();
    m_noiseAmplitude = 0.0;
}

void MeteringEngine::update(double deltaTime)
{
    m_simulationTime += deltaTime;
    
    // Update waveforms
    updateWaveforms(deltaTime);
    
    // Calculate measurements
    calculateMeasurements();
    
    // Process tamper events
    processTamperEvents();
    
    // Update energy measurement
    m_totalEnergy += m_measurements.activePower * deltaTime / 3600.0; // Wh
    m_measurements.energy = m_totalEnergy;
}

void MeteringEngine::updateWaveforms(double deltaTime)
{
    double omega = 2.0 * M_PI * m_configFrequency;
    m_phaseAngle += omega * deltaTime;
    
    if (m_phaseAngle >= 2.0 * M_PI) {
        m_phaseAngle -= 2.0 * M_PI;
    }
    
    // Generate one cycle of samples
    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
        double t = i / SAMPLE_RATE;
        generateSignals(m_simulationTime + t);
    }
}

void MeteringEngine::generateSignals(double time)
{
    double omega = 2.0 * M_PI * m_configFrequency;
    double phase = omega * time;
    
    // Apply signal injections
    double voltageScale = 1.0;
    double frequencyDeviation = 0.0;
    
    for (auto& injection : m_injections) {
        if (injection.active && 
            time >= injection.startTime && 
            time < injection.startTime + injection.duration) {
            
            if (injection.type == "voltage_dip") {
                voltageScale *= (1.0 - injection.magnitude);
            } else if (injection.type == "frequency_variation") {
                frequencyDeviation = injection.magnitude;
            }
        }
    }
    
    // Update frequency with deviation
    double currentFreq = m_configFrequency + frequencyDeviation;
    omega = 2.0 * M_PI * currentFreq;
    phase = omega * time;
    
    // Generate noise
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<> noise(0.0, 1.0);
    
    if (m_isThreePhase) {
        // Three-phase signals
        for (int ph = 0; ph < 3; ph++) {
            double phaseShift = ph * 2.0 * M_PI / 3.0;
            
            // Voltage waveform with harmonics and noise
            double voltage = m_configVoltage * sqrt(2.0) * voltageScale * sin(phase + phaseShift);
            
            // Add harmonics with phase information
            for (const auto& harmonic : m_harmonics) {
                double harmonic_magnitude = harmonic.second.first;
                double harmonic_phase = harmonic.second.second * M_PI / 180.0; // Convert to radians
                voltage += m_configVoltage * sqrt(2.0) * harmonic_magnitude * 
                          sin(harmonic.first * (phase + phaseShift) + harmonic_phase);
            }
            
            // Add interharmonics
            for (const auto& interharm : m_interharmonics) {
                double freq_ratio = interharm.first / m_configFrequency;
                voltage += m_configVoltage * sqrt(2.0) * interharm.second * 
                          sin(freq_ratio * (phase + phaseShift));
            }
            
            // Add noise
            voltage += m_noiseAmplitude * noise(gen);
            
            // Current waveform with power factor
            double powerFactorAngle = acos(m_configPowerFactor);
            double current = m_configCurrent * sqrt(2.0) * sin(phase + phaseShift - powerFactorAngle);
            
            // Apply relay state
            if (!m_relayConnected) {
                current = 0.0;
            }
            
            m_measurements.voltage[ph] = voltage / sqrt(2.0); // RMS
            m_measurements.current[ph] = current / sqrt(2.0); // RMS
        }
    } else {
        // Single-phase signals
        double voltage = m_configVoltage * sqrt(2.0) * voltageScale * sin(phase);
        
        // Add harmonics with phase information
        for (const auto& harmonic : m_harmonics) {
            double harmonic_magnitude = harmonic.second.first;
            double harmonic_phase = harmonic.second.second * M_PI / 180.0; // Convert to radians
            voltage += m_configVoltage * sqrt(2.0) * harmonic_magnitude * 
                      sin(harmonic.first * phase + harmonic_phase);
        }
        
        // Add interharmonics
        for (const auto& interharm : m_interharmonics) {
            double freq_ratio = interharm.first / m_configFrequency;
            voltage += m_configVoltage * sqrt(2.0) * interharm.second * 
                      sin(freq_ratio * phase);
        }
        
        // Add noise
        voltage += m_noiseAmplitude * noise(gen);
        
        // Current waveform with power factor
        double powerFactorAngle = acos(m_configPowerFactor);
        double current = m_configCurrent * sqrt(2.0) * sin(phase - powerFactorAngle);
        
        // Apply relay state
        if (!m_relayConnected) {
            current = 0.0;
        }
        
        // Store in waveform arrays (simplified - using instantaneous values)
        int index = static_cast<int>((phase / (2.0 * M_PI)) * SAMPLES_PER_CYCLE) % SAMPLES_PER_CYCLE;
        m_voltageWaveform[index] = voltage;
        m_currentWaveform[index] = current;
    }
    
    m_measurements.frequency = currentFreq;
}

void MeteringEngine::calculateMeasurements()
{
    if (m_isThreePhase) {
        // Three-phase calculations
        double totalVoltageRMS = 0.0;
        double totalCurrentRMS = 0.0;
        double totalActivePower = 0.0;
        
        for (int ph = 0; ph < 3; ph++) {
            double voltageRMS = std::abs(m_measurements.voltage[ph]);
            double currentRMS = std::abs(m_measurements.current[ph]);
            
            totalVoltageRMS += voltageRMS * voltageRMS;
            totalCurrentRMS += currentRMS * currentRMS;
            totalActivePower += voltageRMS * currentRMS * m_configPowerFactor;
        }
        
        m_measurements.voltageRMS = sqrt(totalVoltageRMS / 3.0);
        m_measurements.currentRMS = sqrt(totalCurrentRMS / 3.0);
        m_measurements.activePower = totalActivePower;
        
    } else {
        // Single-phase calculations
        m_measurements.voltageRMS = m_configVoltage;
        m_measurements.currentRMS = m_relayConnected ? m_configCurrent : 0.0;
        m_measurements.activePower = m_measurements.voltageRMS * m_measurements.currentRMS * m_configPowerFactor;
    }
    
    // Common calculations
    m_measurements.apparentPower = m_measurements.voltageRMS * m_measurements.currentRMS;
    m_measurements.reactivePower = m_measurements.apparentPower * sin(acos(m_configPowerFactor));
    m_measurements.powerFactor = m_configPowerFactor;
    
    // Apply tamper effects
    for (const auto& tamper : m_tamperEvents) {
        if (tamper.second.active) {
            if (tamper.first == "Magnet Tamper") {
                // Magnetic field affects current measurement
                m_measurements.currentRMS *= 0.8;
                m_measurements.activePower *= 0.8;
            } else if (tamper.first == "Reverse Current") {
                // Reverse energy flow
                m_measurements.activePower *= -1.0;
                m_measurements.currentRMS *= -1.0;
            } else if (tamper.first == "Neutral Missing") {
                // Voltage imbalance in 3-phase
                if (m_isThreePhase) {
                    m_measurements.voltage[0] *= 1.2;
                    m_measurements.voltage[1] *= 0.8;
                    m_measurements.voltage[2] *= 0.8;
                }
            } else if (tamper.first == "Phase Loss") {
                // One phase voltage drops to zero
                if (m_isThreePhase) {
                    m_measurements.voltage[0] = 0.0;
                    m_measurements.current[0] = 0.0;
                }
            } else if (tamper.first == "Over Voltage") {
                // Voltage exceeds normal range
                m_measurements.voltageRMS *= 1.3;
            }
        }
    }
    
    // Calculate harmonics and phasors
    calculateHarmonics();
    calculatePhasors();
    calculateCrestFactor();
    calculateKFactor();
    calculatePowerFactorComponents();
    
    // Calculate THD from harmonic data
    double thd_v_squared = 0.0;
    double thd_i_squared = 0.0;
    
    for (int h = 1; h < 33; h++) { // 2nd to 33rd harmonic
        thd_v_squared += m_measurements.voltageHarmonics[h].magnitude * m_measurements.voltageHarmonics[h].magnitude;
        thd_i_squared += m_measurements.currentHarmonics[h].magnitude * m_measurements.currentHarmonics[h].magnitude;
    }
    
    double fundamental_v = m_measurements.voltageHarmonics[0].magnitude; // 1st harmonic (fundamental)
    double fundamental_i = m_measurements.currentHarmonics[0].magnitude;
    
    m_measurements.thd_voltage = (fundamental_v > 0) ? sqrt(thd_v_squared) / fundamental_v * 100.0 : 0.0;
    m_measurements.thd_current = (fundamental_i > 0) ? sqrt(thd_i_squared) / fundamental_i * 100.0 : 0.0;
}

void MeteringEngine::processTamperEvents()
{
    // Check for automatic tamper detection based on measurements
    
    // Over/under voltage detection
    if (m_measurements.voltageRMS > m_configVoltage * 1.1) {
        if (m_tamperEvents.find("Over Voltage") == m_tamperEvents.end()) {
            injectTamperEvent("Over Voltage");
        }
    } else if (m_measurements.voltageRMS < m_configVoltage * 0.9) {
        if (m_tamperEvents.find("Under Voltage") == m_tamperEvents.end()) {
            injectTamperEvent("Under Voltage");
        }
    }
    
    // Frequency deviation detection
    if (std::abs(m_measurements.frequency - 50.0) > 1.0) {
        if (m_tamperEvents.find("Frequency Deviation") == m_tamperEvents.end()) {
            injectTamperEvent("Frequency Deviation");
        }
    }
    
    // Reverse power flow detection
    if (m_measurements.activePower < -10.0) {
        if (m_tamperEvents.find("Reverse Power Flow") == m_tamperEvents.end()) {
            injectTamperEvent("Reverse Power Flow");
        }
    }
}

void MeteringEngine::injectTamperEvent(const std::string& type)
{
    TamperEvent event;
    event.type = type;
    event.timestamp = std::chrono::system_clock::now();
    event.active = true;
    
    m_tamperEvents[type] = event;
    
    std::cout << "Tamper event injected: " << type << std::endl;
}

void MeteringEngine::clearTamperEvent(const std::string& type)
{
    auto it = m_tamperEvents.find(type);
    if (it != m_tamperEvents.end()) {
        it->second.active = false;
    }
}

std::vector<TamperEvent> MeteringEngine::getActiveTamperEvents() const
{
    std::vector<TamperEvent> activeEvents;
    for (const auto& tamper : m_tamperEvents) {
        if (tamper.second.active) {
            activeEvents.push_back(tamper.second);
        }
    }
    return activeEvents;
}

std::vector<double> MeteringEngine::getVoltageWaveform() const
{
    return m_voltageWaveform;
}

std::vector<double> MeteringEngine::getCurrentWaveform() const
{
    return m_currentWaveform;
}

void MeteringEngine::injectVoltageDip(double magnitude, double duration)
{
    SignalInjection injection;
    injection.active = true;
    injection.startTime = m_simulationTime;
    injection.duration = duration;
    injection.magnitude = magnitude;
    injection.type = "voltage_dip";
    
    m_injections.push_back(injection);
}

void MeteringEngine::injectFrequencyVariation(double deviation, double duration)
{
    SignalInjection injection;
    injection.active = true;
    injection.startTime = m_simulationTime;
    injection.duration = duration;
    injection.magnitude = deviation;
    injection.type = "frequency_variation";
    
    m_injections.push_back(injection);
}

void MeteringEngine::injectHarmonics(int harmonic, double magnitude, double phase)
{
    if (harmonic >= 1 && harmonic <= 33) {
        m_harmonics[harmonic] = std::make_pair(magnitude, phase);
    }
}

void MeteringEngine::injectInterharmonics(double frequency, double magnitude)
{
    m_interharmonics[frequency] = magnitude;
}

void MeteringEngine::injectNoise(double amplitude)
{
    m_noiseAmplitude = amplitude;
}

double MeteringEngine::calculateRMS(const std::vector<double>& samples)
{
    double sum = 0.0;
    for (double sample : samples) {
        sum += sample * sample;
    }
    return sqrt(sum / samples.size());
}

double MeteringEngine::calculateTHD(const std::vector<double>& samples)
{
    // Simplified THD calculation
    // In a real implementation, this would use FFT
    return 0.0;
}

void MeteringEngine::calculateHarmonics()
{
    // Simulate harmonic analysis based on injected harmonics
    // In a real implementation, this would use FFT on the waveform samples
    
    // Set fundamental (1st harmonic)
    m_measurements.voltageHarmonics[0].magnitude = m_measurements.voltageRMS;
    m_measurements.voltageHarmonics[0].phase = 0.0;
    m_measurements.voltageHarmonics[0].percentage = 100.0;
    
    m_measurements.currentHarmonics[0].magnitude = m_measurements.currentRMS;
    m_measurements.currentHarmonics[0].phase = -acos(m_configPowerFactor) * 180.0 / M_PI;
    m_measurements.currentHarmonics[0].percentage = 100.0;
    
    // Set harmonics based on injected values
    for (int h = 1; h < 33; h++) {
        int harmonic_order = h + 1; // h=0 is fundamental, h=1 is 2nd harmonic, etc.
        
        if (m_harmonics.find(harmonic_order) != m_harmonics.end()) {
            auto harmonic_data = m_harmonics[harmonic_order];
            
            m_measurements.voltageHarmonics[h].magnitude = m_measurements.voltageRMS * harmonic_data.first;
            m_measurements.voltageHarmonics[h].phase = harmonic_data.second;
            m_measurements.voltageHarmonics[h].percentage = harmonic_data.first * 100.0;
            
            m_measurements.currentHarmonics[h].magnitude = m_measurements.currentRMS * harmonic_data.first;
            m_measurements.currentHarmonics[h].phase = harmonic_data.second;
            m_measurements.currentHarmonics[h].percentage = harmonic_data.first * 100.0;
        } else {
            m_measurements.voltageHarmonics[h] = {0.0, 0.0, 0.0};
            m_measurements.currentHarmonics[h] = {0.0, 0.0, 0.0};
        }
    }
}

void MeteringEngine::calculatePhasors()
{
    if (m_isThreePhase) {
        // Three-phase phasors
        for (int ph = 0; ph < 3; ph++) {
            double phase_shift = ph * 120.0; // 120 degrees phase shift
            
            // Voltage phasor
            m_measurements.voltagePhasor[ph].magnitude = std::abs(m_measurements.voltage[ph]);
            m_measurements.voltagePhasor[ph].phase = phase_shift;
            m_measurements.voltagePhasor[ph].real = m_measurements.voltagePhasor[ph].magnitude * 
                                                   cos(phase_shift * M_PI / 180.0);
            m_measurements.voltagePhasor[ph].imag = m_measurements.voltagePhasor[ph].magnitude * 
                                                   sin(phase_shift * M_PI / 180.0);
            
            // Current phasor (with power factor lag)
            double current_phase = phase_shift - acos(m_configPowerFactor) * 180.0 / M_PI;
            m_measurements.currentPhasor[ph].magnitude = std::abs(m_measurements.current[ph]);
            m_measurements.currentPhasor[ph].phase = current_phase;
            m_measurements.currentPhasor[ph].real = m_measurements.currentPhasor[ph].magnitude * 
                                                   cos(current_phase * M_PI / 180.0);
            m_measurements.currentPhasor[ph].imag = m_measurements.currentPhasor[ph].magnitude * 
                                                   sin(current_phase * M_PI / 180.0);
        }
    } else {
        // Single-phase phasors
        m_measurements.voltagePhasor[0].magnitude = m_measurements.voltageRMS;
        m_measurements.voltagePhasor[0].phase = 0.0;
        m_measurements.voltagePhasor[0].real = m_measurements.voltageRMS;
        m_measurements.voltagePhasor[0].imag = 0.0;
        
        double current_phase = -acos(m_configPowerFactor) * 180.0 / M_PI;
        m_measurements.currentPhasor[0].magnitude = m_measurements.currentRMS;
        m_measurements.currentPhasor[0].phase = current_phase;
        m_measurements.currentPhasor[0].real = m_measurements.currentRMS * cos(current_phase * M_PI / 180.0);
        m_measurements.currentPhasor[0].imag = m_measurements.currentRMS * sin(current_phase * M_PI / 180.0);
        
        // Clear unused phases
        for (int ph = 1; ph < 3; ph++) {
            m_measurements.voltagePhasor[ph] = {0.0, 0.0, 0.0, 0.0};
            m_measurements.currentPhasor[ph] = {0.0, 0.0, 0.0, 0.0};
        }
    }
}

void MeteringEngine::calculateCrestFactor()
{
    // Simplified crest factor calculation
    // Crest factor = Peak value / RMS value
    double voltage_peak = m_measurements.voltageRMS * sqrt(2.0);
    double current_peak = m_measurements.currentRMS * sqrt(2.0);
    
    // Add effect of harmonics to peak value
    for (int h = 1; h < 33; h++) {
        voltage_peak += m_measurements.voltageHarmonics[h].magnitude * sqrt(2.0);
        current_peak += m_measurements.currentHarmonics[h].magnitude * sqrt(2.0);
    }
    
    m_measurements.crest_factor_voltage = (m_measurements.voltageRMS > 0) ? voltage_peak / m_measurements.voltageRMS : 0.0;
    m_measurements.crest_factor_current = (m_measurements.currentRMS > 0) ? current_peak / m_measurements.currentRMS : 0.0;
}

void MeteringEngine::calculateKFactor()
{
    // K-factor calculation for transformer derating
    double k_factor = 1.0; // Fundamental contributes 1
    
    for (int h = 1; h < 33; h++) {
        int harmonic_order = h + 1;
        double harmonic_percentage = m_measurements.currentHarmonics[h].percentage / 100.0;
        k_factor += harmonic_order * harmonic_order * harmonic_percentage * harmonic_percentage;
    }
    
    m_measurements.k_factor = k_factor;
}

void MeteringEngine::calculatePowerFactorComponents()
{
    // Displacement power factor (fundamental component only)
    m_measurements.displacement_pf = m_configPowerFactor;
    
    // Distortion power factor (effect of harmonics)
    double total_rms_squared = m_measurements.currentHarmonics[0].magnitude * m_measurements.currentHarmonics[0].magnitude;
    double harmonic_rms_squared = 0.0;
    
    for (int h = 1; h < 33; h++) {
        double harmonic_rms = m_measurements.currentHarmonics[h].magnitude;
        harmonic_rms_squared += harmonic_rms * harmonic_rms;
        total_rms_squared += harmonic_rms * harmonic_rms;
    }
    
    double fundamental_rms = m_measurements.currentHarmonics[0].magnitude;
    m_measurements.distortion_pf = (total_rms_squared > 0) ? fundamental_rms / sqrt(total_rms_squared) : 1.0;
}

std::vector<HarmonicData> MeteringEngine::getVoltageHarmonics() const
{
    std::vector<HarmonicData> harmonics;
    for (int i = 0; i < 33; i++) {
        harmonics.push_back(m_measurements.voltageHarmonics[i]);
    }
    return harmonics;
}

std::vector<HarmonicData> MeteringEngine::getCurrentHarmonics() const
{
    std::vector<HarmonicData> harmonics;
    for (int i = 0; i < 33; i++) {
        harmonics.push_back(m_measurements.currentHarmonics[i]);
    }
    return harmonics;
}

std::vector<PhasorData> MeteringEngine::getVoltagePhasors() const
{
    std::vector<PhasorData> phasors;
    for (int i = 0; i < 3; i++) {
        phasors.push_back(m_measurements.voltagePhasor[i]);
    }
    return phasors;
}

std::vector<PhasorData> MeteringEngine::getCurrentPhasors() const
{
    std::vector<PhasorData> phasors;
    for (int i = 0; i < 3; i++) {
        phasors.push_back(m_measurements.currentPhasor[i]);
    }
    return phasors;
}
