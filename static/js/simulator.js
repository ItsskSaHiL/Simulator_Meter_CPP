
// Simulator API Interface
class SimulatorAPI {
    constructor() {
        this.baseURL = '';
        this.updateInterval = null;
    }

    async getStatus() {
        try {
            const response = await fetch('/api/status');
            return await response.json();
        } catch (error) {
            console.error('Error getting status:', error);
            return null;
        }
    }

    async loadFirmware(file) {
        const formData = new FormData();
        formData.append('file', file);

        try {
            const response = await fetch('/api/firmware/load', {
                method: 'POST',
                body: formData
            });
            return await response.json();
        } catch (error) {
            console.error('Error loading firmware:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async startSimulation() {
        try {
            const response = await fetch('/api/simulation/start', {
                method: 'POST'
            });
            return await response.json();
        } catch (error) {
            console.error('Error starting simulation:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async stopSimulation() {
        try {
            const response = await fetch('/api/simulation/stop', {
                method: 'POST'
            });
            return await response.json();
        } catch (error) {
            console.error('Error stopping simulation:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async resetSimulation() {
        try {
            const response = await fetch('/api/simulation/reset', {
                method: 'POST'
            });
            return await response.json();
        } catch (error) {
            console.error('Error resetting simulation:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async updateConfiguration(config) {
        try {
            const response = await fetch('/api/configuration/update', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(config)
            });
            return await response.json();
        } catch (error) {
            console.error('Error updating configuration:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async injectTamper(type) {
        try {
            const response = await fetch('/api/tamper/inject', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ type })
            });
            return await response.json();
        } catch (error) {
            console.error('Error injecting tamper:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async sendProtocolCommand(protocol, command) {
        try {
            const response = await fetch('/api/protocol/send', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ protocol, command })
            });
            return await response.json();
        } catch (error) {
            console.error('Error sending protocol command:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async sendUartCommand(command) {
        try {
            const response = await fetch('/api/uart/send', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ command })
            });
            return await response.json();
        } catch (error) {
            console.error('Error sending UART command:', error);
            return { success: false, message: 'Network error' };
        }
    }

    async getWaveformData() {
        try {
            const response = await fetch('/api/waveform');
            return await response.json();
        } catch (error) {
            console.error('Error getting waveform data:', error);
            return null;
        }
    }

    async getProtocolLogs() {
        try {
            const response = await fetch('/api/logs/protocol');
            return await response.json();
        } catch (error) {
            console.error('Error getting protocol logs:', error);
            return [];
        }
    }

    async getUartLogs() {
        try {
            const response = await fetch('/api/logs/uart');
            return await response.json();
        } catch (error) {
            console.error('Error getting UART logs:', error);
            return [];
        }
    }

    async exportCSV() {
        try {
            const response = await fetch('/api/export/csv');
            const blob = await response.blob();
            
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `simulation_data_${new Date().toISOString().split('T')[0]}.csv`;
            a.click();
            window.URL.revokeObjectURL(url);
            
            return { success: true };
        } catch (error) {
            console.error('Error exporting CSV:', error);
            return { success: false, message: 'Export failed' };
        }
    }

    startUpdates(callback, interval = 1000) {
        this.stopUpdates();
        this.updateInterval = setInterval(async () => {
            const status = await this.getStatus();
            if (status && callback) {
                callback(status);
            }
        }, interval);
    }

    stopUpdates() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
        }
    }
}

// Extend the main simulator class with API integration
if (window.SmartMeterSimulator) {
    const originalClass = window.SmartMeterSimulator;
    
    window.SmartMeterSimulator = class extends originalClass {
        constructor() {
            super();
            this.api = new SimulatorAPI();
            this.setupAPIIntegration();
        }

        setupAPIIntegration() {
            // Start status updates from server
            this.api.startUpdates((status) => {
                this.updateFromServer(status);
            }, 500);
        }

        updateFromServer(status) {
            if (!status) return;

            // Update local state
            this.isRunning = status.is_running;
            
            // Update measurements display
            if (status.measurements) {
                Object.assign(this.simulationData.measurements, status.measurements);
                this.updateMeasurementsDisplay();
            }

            // Update peripherals
            if (status.peripherals) {
                this.updatePeripheralsFromServer(status.peripherals);
            }

            // Update UI state
            this.updateUIState(status);
        }

        updateUIState(status) {
            const startBtn = document.getElementById('startSimulation');
            const stopBtn = document.getElementById('stopSimulation');
            const statusElement = document.getElementById('simulationStatus');

            if (status.is_running) {
                startBtn.disabled = true;
                stopBtn.disabled = false;
                statusElement.textContent = 'Running';
                statusElement.className = 'status-badge status-running';
            } else {
                startBtn.disabled = !status.firmware_loaded;
                stopBtn.disabled = true;
                statusElement.textContent = status.firmware_loaded ? 'Ready' : 'No Firmware';
                statusElement.className = `status-badge ${status.firmware_loaded ? 'status-paused' : 'status-stopped'}`;
            }
        }

        updatePeripheralsFromServer(peripherals) {
            // Update GPIO
            if (peripherals.gpio) {
                peripherals.gpio.forEach(gpio => {
                    const stateElement = document.getElementById(`gpio${gpio.pin}State`);
                    if (stateElement) {
                        stateElement.textContent = gpio.state ? 'HIGH' : 'LOW';
                    }
                });
            }

            // Update ADC
            if (peripherals.adc) {
                peripherals.adc.forEach(adc => {
                    const valueElement = document.getElementById(`adc${adc.channel}Value`);
                    if (valueElement) {
                        valueElement.textContent = `${adc.voltage.toFixed(3)}V`;
                    }
                });
            }

            // Update Timers
            if (peripherals.timers) {
                peripherals.timers.forEach(timer => {
                    const statusElement = document.getElementById(`timer${timer.timer}Status`);
                    if (statusElement) {
                        statusElement.textContent = timer.enabled ? 'Running' : 'Stopped';
                    }
                });
            }
        }

        async processFirmwareFile(file) {
            const result = await this.api.loadFirmware(file);
            
            if (result.success) {
                this.logMessage(`Firmware file loaded: ${file.name}`, 'info');
                document.getElementById('startSimulation').disabled = false;
                this.closeFirmwareModal();
            } else {
                this.logMessage(`Error: ${result.message}`, 'error');
            }
        }

        async startSimulation() {
            const result = await this.api.startSimulation();
            
            if (result.success) {
                this.logMessage('Simulation started', 'info');
            } else {
                this.logMessage(`Error: ${result.message}`, 'error');
            }
        }

        async stopSimulation() {
            const result = await this.api.stopSimulation();
            
            if (result.success) {
                this.logMessage('Simulation stopped', 'info');
            } else {
                this.logMessage(`Error: ${result.message}`, 'error');
            }
        }

        async resetSimulation() {
            const result = await this.api.resetSimulation();
            
            if (result.success) {
                this.logMessage('Simulation reset', 'info');
                this.updateProgress(0);
            } else {
                this.logMessage(`Error: ${result.message}`, 'error');
            }
        }

        async updateVoltage(value) {
            await this.api.updateConfiguration({ voltage: parseFloat(value) });
            super.updateVoltage(value);
        }

        async updateCurrent(value) {
            await this.api.updateConfiguration({ current: parseFloat(value) });
            super.updateCurrent(value);
        }

        async updateFrequency(value) {
            await this.api.updateConfiguration({ frequency: parseFloat(value) });
            super.updateFrequency(value);
        }

        async updatePowerFactor(value) {
            await this.api.updateConfiguration({ power_factor: parseFloat(value) });
            super.updatePowerFactor(value);
        }

        async injectTamperEvent() {
            const tamperType = document.getElementById('tamperType').value;
            const result = await this.api.injectTamper(tamperType);
            
            if (result.success) {
                this.logMessage(`Tamper event injected: ${tamperType}`, 'warning');
            } else {
                this.logMessage(`Error: ${result.message}`, 'error');
            }
        }

        async sendUartCommand() {
            const input = document.getElementById('uartInput');
            const command = input.value.trim();
            
            if (command) {
                const result = await this.api.sendUartCommand(command);
                
                if (result.success) {
                    this.addToTerminal(`TX: ${command}`);
                    this.addToTerminal(`RX: ${result.response}`);
                } else {
                    this.addToTerminal(`Error: ${result.message}`);
                }
                
                input.value = '';
            }
        }

        async sendProtocolCommand() {
            const input = document.getElementById('protocolCommand');
            const command = input.value.trim();
            const protocol = document.getElementById('protocolType').value;
            
            if (command) {
                const result = await this.api.sendProtocolCommand(protocol, command);
                
                if (result.success) {
                    this.logProtocol(`[${protocol.toUpperCase()}] TX: ${command}`);
                    this.logProtocol(`[${protocol.toUpperCase()}] RX: ${result.response}`);
                    document.getElementById('protocolResponse').textContent = result.response;
                } else {
                    this.logProtocol(`Error: ${result.message}`);
                }
                
                input.value = '';
            }
        }

        async exportLog() {
            const result = await this.api.exportCSV();
            
            if (result.success) {
                this.logMessage('Data exported successfully', 'info');
            } else {
                this.logMessage(`Export failed: ${result.message}`, 'error');
            }
        }

        async updateCharts() {
            // Get real waveform data from server
            const waveformData = await this.api.getWaveformData();
            
            if (waveformData) {
                // Update waveform chart
                this.waveformChart.data.datasets[0].data = waveformData.voltage;
                this.waveformChart.data.datasets[1].data = waveformData.current;
                this.waveformChart.update('none');
                
                // Update oscilloscope chart
                this.oscilloscopeChart.data.datasets[0].data = waveformData.voltage.slice(0, 200);
                this.oscilloscopeChart.data.datasets[1].data = waveformData.current.slice(0, 200);
                this.oscilloscopeChart.update('none');
            }
            
            // Update performance chart (keep existing implementation)
            super.updateCharts();
        }

        startUpdateLoop() {
            // Override to use server updates instead of local simulation
            setInterval(() => {
                if (this.isRunning) {
                    // Only update charts and UI, data comes from server
                    this.updateCharts();
                    const progress = (Date.now() % 10000) / 100;
                    this.updateProgress(progress);
                }
            }, 100);
        }
    };
}

// Test automation features
class TestAutomation {
    constructor(simulator) {
        this.simulator = simulator;
        this.testScripts = [];
        this.isRunning = false;
    }

    createTestScript(name, steps) {
        const script = {
            id: Date.now(),
            name,
            steps,
            created: new Date().toISOString(),
            lastRun: null,
            results: []
        };
        
        this.testScripts.push(script);
        this.updateTestScriptsList();
        return script;
    }

    async runTestScript(scriptId) {
        const script = this.testScripts.find(s => s.id === scriptId);
        if (!script) return;

        this.isRunning = true;
        script.lastRun = new Date().toISOString();
        
        const results = [];
        
        for (const step of script.steps) {
            try {
                const result = await this.executeTestStep(step);
                results.push({ step, result, success: true });
            } catch (error) {
                results.push({ step, result: error.message, success: false });
                break;
            }
        }
        
        script.results = results;
        this.isRunning = false;
        this.updateTestScriptsList();
        
        return results;
    }

    async executeTestStep(step) {
        switch (step.type) {
            case 'setVoltage':
                document.getElementById('voltageInput').value = step.value;
                await this.simulator.updateVoltage(step.value);
                break;
                
            case 'setCurrent':
                document.getElementById('currentInput').value = step.value;
                await this.simulator.updateCurrent(step.value);
                break;
                
            case 'injectTamper':
                document.getElementById('tamperType').value = step.tamperType;
                await this.simulator.injectTamperEvent();
                break;
                
            case 'sendProtocol':
                document.getElementById('protocolType').value = step.protocol;
                document.getElementById('protocolCommand').value = step.command;
                await this.simulator.sendProtocolCommand();
                break;
                
            case 'wait':
                await new Promise(resolve => setTimeout(resolve, step.duration));
                break;
                
            case 'assert':
                const actualValue = this.getValueByPath(step.path);
                if (actualValue !== step.expectedValue) {
                    throw new Error(`Assertion failed: expected ${step.expectedValue}, got ${actualValue}`);
                }
                break;
        }
    }

    getValueByPath(path) {
        // Get value from simulator data by path (e.g., 'measurements.voltageRMS')
        const parts = path.split('.');
        let value = this.simulator.simulationData;
        
        for (const part of parts) {
            value = value[part];
            if (value === undefined) return null;
        }
        
        return value;
    }

    updateTestScriptsList() {
        const container = document.getElementById('testScriptsList');
        if (!container) return;

        container.innerHTML = '';
        
        this.testScripts.forEach(script => {
            const item = document.createElement('div');
            item.className = 'test-script-item';
            
            const lastRun = script.lastRun ? new Date(script.lastRun).toLocaleString() : 'Never';
            const status = script.results.length > 0 ? 
                (script.results.every(r => r.success) ? 'Passed' : 'Failed') : 'Not run';
            
            item.innerHTML = `
                <div>
                    <strong>${script.name}</strong>
                    <div style="font-size: 0.8rem; color: #666;">
                        Last run: ${lastRun} | Status: ${status}
                    </div>
                </div>
                <div>
                    <button class="btn btn-primary btn-sm" onclick="testAutomation.runTestScript(${script.id})">
                        Run
                    </button>
                    <button class="btn btn-danger btn-sm" onclick="testAutomation.deleteTestScript(${script.id})">
                        Delete
                    </button>
                </div>
            `;
            
            container.appendChild(item);
        });
    }

    deleteTestScript(scriptId) {
        this.testScripts = this.testScripts.filter(s => s.id !== scriptId);
        this.updateTestScriptsList();
    }
}

// Initialize test automation when simulator is ready
document.addEventListener('DOMContentLoaded', () => {
    setTimeout(() => {
        if (window.simulator) {
            window.testAutomation = new TestAutomation(window.simulator);
            
            // Setup test automation buttons
            document.getElementById('createTestScript')?.addEventListener('click', () => {
                const name = prompt('Enter test script name:');
                if (name) {
                    const script = window.testAutomation.createTestScript(name, [
                        { type: 'setVoltage', value: 240 },
                        { type: 'setCurrent', value: 10 },
                        { type: 'wait', duration: 1000 },
                        { type: 'assert', path: 'measurements.voltageRMS', expectedValue: 240 }
                    ]);
                    
                    window.simulator.logMessage(`Test script created: ${script.name}`, 'info');
                }
            });
            
            document.getElementById('runTestSuite')?.addEventListener('click', async () => {
                for (const script of window.testAutomation.testScripts) {
                    await window.testAutomation.runTestScript(script.id);
                }
                window.simulator.logMessage('Test suite completed', 'info');
            });
        }
    }, 1000);
});
