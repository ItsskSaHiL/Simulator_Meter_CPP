
#include "simulator_core.h"
#include "mcu_emulator.h"
#include "metering_engine.h"
#include "protocol_handler.h"
#include <iostream>

SimulatorCore::SimulatorCore()
    : m_lastUpdate(std::chrono::high_resolution_clock::now())
{
}

SimulatorCore::~SimulatorCore()
{
    stopSimulation();
}

void SimulatorCore::startSimulation()
{
    if (m_running) return;
    
    m_running = true;
    m_paused = false;
    m_lastUpdate = std::chrono::high_resolution_clock::now();
    
    m_simulationThread = std::make_unique<std::thread>(&SimulatorCore::simulationLoop, this);
}

void SimulatorCore::stopSimulation()
{
    m_running = false;
    if (m_simulationThread && m_simulationThread->joinable()) {
        m_simulationThread->join();
    }
}

void SimulatorCore::pauseSimulation()
{
    m_paused = !m_paused;
}

void SimulatorCore::resetSimulation()
{
    stopSimulation();
    
    if (m_mcuEmulator) {
        m_mcuEmulator->reset();
    }
    if (m_meteringEngine) {
        m_meteringEngine->reset();
    }
}

void SimulatorCore::setMCUEmulator(std::shared_ptr<MCUEmulator> emulator)
{
    m_mcuEmulator = emulator;
}

void SimulatorCore::setMeteringEngine(std::shared_ptr<MeteringEngine> engine)
{
    m_meteringEngine = engine;
}

void SimulatorCore::setProtocolHandler(std::shared_ptr<ProtocolHandler> handler)
{
    m_protocolHandler = handler;
}

void SimulatorCore::simulationLoop()
{
    const auto frameDuration = std::chrono::microseconds(1000000 / SIMULATION_FREQUENCY_HZ);
    
    while (m_running) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        if (!m_paused) {
            updateComponents();
        }
        
        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto elapsed = frameEnd - frameStart;
        
        if (elapsed < frameDuration) {
            std::this_thread::sleep_for(frameDuration - elapsed);
        }
    }
}

void SimulatorCore::updateComponents()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastUpdate).count() / 1000000.0;
    m_lastUpdate = now;
    
    // Update metering engine first (provides ADC values)
    if (m_meteringEngine) {
        m_meteringEngine->update(deltaTime);
    }
    
    // Update MCU emulator (processes ADC values, runs firmware)
    if (m_mcuEmulator) {
        m_mcuEmulator->update(deltaTime);
    }
    
    // Update protocol handler (processes communication)
    if (m_protocolHandler) {
        m_protocolHandler->update(deltaTime);
    }
}
