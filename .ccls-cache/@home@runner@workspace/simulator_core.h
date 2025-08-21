
#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

class MCUEmulator;
class MeteringEngine;
class ProtocolHandler;

class SimulatorCore
{
public:
    SimulatorCore();
    ~SimulatorCore();

    void startSimulation();
    void stopSimulation();
    void pauseSimulation();
    void resetSimulation();
    
    bool isRunning() const { return m_running; }
    
    void setMCUEmulator(std::shared_ptr<MCUEmulator> emulator);
    void setMeteringEngine(std::shared_ptr<MeteringEngine> engine);
    void setProtocolHandler(std::shared_ptr<ProtocolHandler> handler);

private:
    void simulationLoop();
    void updateComponents();

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    std::unique_ptr<std::thread> m_simulationThread;
    
    std::shared_ptr<MCUEmulator> m_mcuEmulator;
    std::shared_ptr<MeteringEngine> m_meteringEngine;
    std::shared_ptr<ProtocolHandler> m_protocolHandler;
    
    std::chrono::high_resolution_clock::time_point m_lastUpdate;
    static constexpr int SIMULATION_FREQUENCY_HZ = 1000;
};
