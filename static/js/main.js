// Main Application Controller
class SmartMeterSimulator {
    constructor() {
        this.isRunning = false;
        this.currentTab = 'firmware';
        this.simulationData = {
            voltage: 230,
            current: 5,
            frequency: 50,
            powerFactor: 0.95,
            measurements: {
                voltageRMS: 0,
                currentRMS: 0,
                activePower: 0,
                reactivePower: 0,
                apparentPower: 0,
                powerFactor: 0,
                frequency: 0,
                energy: 0
            },
            harmonics: {
                voltage: [],
                current: []
            },
            phasors: {
                voltage: { real: 0, imag: 0 },
                current: { real: 0, imag: 0 }
            }
        };

        this.init();
    }

    init() {
        this.setupEventListeners();
        this.setupTabs();
        this.initializeCharts();
        this.setupCircuitDesign();
        this.startUpdateLoop();
    }

    setupEventListeners() {
        // Header controls
        document.getElementById('loadFirmware').addEventListener('click', () => this.showFirmwareModal());
        document.getElementById('startSimulation').addEventListener('click', () => this.startSimulation());
        document.getElementById('stopSimulation').addEventListener('click', () => this.stopSimulation());
        document.getElementById('resetSimulation').addEventListener('click', () => this.resetSimulation());

        // Firmware file upload
        document.getElementById('firmwareFile').addEventListener('change', (e) => this.handleFirmwareUpload(e));

        // Metering controls
        document.getElementById('voltageInput').addEventListener('input', (e) => this.updateVoltage(e.target.value));
        document.getElementById('currentInput').addEventListener('input', (e) => this.updateCurrent(e.target.value));
        document.getElementById('frequencyInput').addEventListener('input', (e) => this.updateFrequency(e.target.value));
        document.getElementById('powerFactorInput').addEventListener('input', (e) => this.updatePowerFactor(e.target.value));
        document.getElementById('injectTamper').addEventListener('click', () => this.injectTamperEvent());

        // UART terminal
        document.getElementById('sendUart').addEventListener('click', () => this.sendUartCommand());
        document.getElementById('uartInput').addEventListener('keypress', (e) => {
            if (e.key === 'Enter') this.sendUartCommand();
        });

        // Protocol interface
        document.getElementById('sendProtocolCommand').addEventListener('click', () => this.sendProtocolCommand());
        document.getElementById('protocolCommand').addEventListener('keypress', (e) => {
            if (e.key === 'Enter') this.sendProtocolCommand();
        });

        // Log controls
        document.getElementById('exportLog').addEventListener('click', () => this.exportLog());
        document.getElementById('clearLog').addEventListener('click', () => this.clearLog());

        // Modal controls
        document.querySelector('.close').addEventListener('click', () => this.closeFirmwareModal());
        document.getElementById('fileDropZone').addEventListener('click', () => {
            document.getElementById('firmwareFileInput').click();
        });
        document.getElementById('firmwareFileInput').addEventListener('change', (e) => this.handleFirmwareUpload(e));

        // MCU configuration
        document.getElementById('mcuFamily').addEventListener('change', () => this.loadMCUPartNumbers());
        document.getElementById('mcuPartNumber').addEventListener('change', () => this.updateMCUConfiguration());
        document.getElementById('mcuArchitecture').addEventListener('change', () => this.updateMCUConfiguration());

        // Drag and drop
        this.setupDragAndDrop();
    }

    setupTabs() {
        const navItems = document.querySelectorAll('.nav-item');
        const tabBtns = document.querySelectorAll('.tab-btn');
        
        // Handle sidebar navigation
        navItems.forEach(item => {
            item.addEventListener('click', () => {
                const tabName = item.dataset.tab;
                this.switchTab(tabName);
            });
        });
        
        // Handle tab buttons (if they exist)
        tabBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                const tabName = btn.dataset.tab;
                this.switchTab(tabName);
            });
        });
    }

    switchTab(tabName) {
        // Update navigation items
        document.querySelectorAll('.nav-item').forEach(item => {
            item.classList.remove('active');
            if (item.dataset.tab === tabName) {
                item.classList.add('active');
            }
        });
        
        // Update tab buttons
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.classList.remove('active');
            if (btn.dataset.tab === tabName) {
                btn.classList.add('active');
            }
        });

        // Update content
        document.querySelectorAll('.tab-content').forEach(content => {
            content.classList.remove('active');
        });
        
        const targetTab = document.getElementById(`${tabName}-tab`);
        if (targetTab) {
            targetTab.classList.add('active');
        }

        this.currentTab = tabName;
        
        // Initialize specific tab functionality
        this.initializeTabContent(tabName);
    }

    showFirmwareModal() {
        document.getElementById('firmwareModal').style.display = 'block';
    }

    closeFirmwareModal() {
        document.getElementById('firmwareModal').style.display = 'none';
    }

    setupDragAndDrop() {
        const dropZone = document.getElementById('fileDropZone');

        dropZone.addEventListener('dragover', (e) => {
            e.preventDefault();
            dropZone.style.background = '#e3f2fd';
        });

        dropZone.addEventListener('dragleave', () => {
            dropZone.style.background = '#f8f9fa';
        });

        dropZone.addEventListener('drop', (e) => {
            e.preventDefault();
            dropZone.style.background = '#f8f9fa';

            const files = e.dataTransfer.files;
            if (files.length > 0) {
                this.processFirmwareFile(files[0]);
            }
        });
    }

    handleFirmwareUpload(event) {
        const file = event.target.files[0];
        if (file) {
            this.processFirmwareFile(file);
        }
    }

    processFirmwareFile(file) {
        const validExtensions = ['.hex', '.bin'];
        const fileName = file.name.toLowerCase();
        const isValid = validExtensions.some(ext => fileName.endsWith(ext));

        if (!isValid) {
            this.logMessage('Error: Invalid firmware file format. Please select a .hex or .bin file.', 'error');
            return;
        }

        this.logMessage(`Firmware file loaded: ${file.name}`, 'info');
        document.getElementById('startSimulation').disabled = false;
        this.closeFirmwareModal();

        // Update status
        document.getElementById('simulationStatus').textContent = 'Firmware Loaded';
        document.getElementById('simulationStatus').className = 'status-badge status-paused';
    }

    startSimulation() {
        if (this.isRunning) return;

        this.isRunning = true;
        document.getElementById('startSimulation').disabled = true;
        document.getElementById('stopSimulation').disabled = false;
        document.getElementById('simulationStatus').textContent = 'Running';
        document.getElementById('simulationStatus').className = 'status-badge status-running';

        this.logMessage('Simulation started', 'info');
        this.updateProgress(0);
    }

    stopSimulation() {
        if (!this.isRunning) return;

        this.isRunning = false;
        document.getElementById('startSimulation').disabled = false;
        document.getElementById('stopSimulation').disabled = true;
        document.getElementById('simulationStatus').textContent = 'Stopped';
        document.getElementById('simulationStatus').className = 'status-badge status-stopped';

        this.logMessage('Simulation stopped', 'info');
    }

    resetSimulation() {
        this.stopSimulation();
        this.simulationData.measurements = {
            voltageRMS: 0,
            currentRMS: 0,
            activePower: 0,
            reactivePower: 0,
            apparentPower: 0,
            powerFactor: 0,
            frequency: 0,
            energy: 0
        };
        this.simulationData.harmonics = { voltage: [], current: [] };
        this.simulationData.phasors = { voltage: { real: 0, imag: 0 }, current: { real: 0, imag: 0 } };
        this.updateProgress(0);
        this.logMessage('Simulation reset', 'info');
    }

    updateVoltage(value) {
        this.simulationData.voltage = parseFloat(value);
    }

    updateCurrent(value) {
        this.simulationData.current = parseFloat(value);
    }

    updateFrequency(value) {
        this.simulationData.frequency = parseFloat(value);
    }

    updatePowerFactor(value) {
        this.simulationData.powerFactor = parseFloat(value);
    }

    injectTamperEvent() {
        const tamperType = document.getElementById('tamperType').value;
        this.logMessage(`Tamper event injected: ${tamperType}`, 'warning');
    }

    sendUartCommand() {
        const input = document.getElementById('uartInput');
        const command = input.value.trim();

        if (command) {
            this.addToTerminal(`TX: ${command}`);
            input.value = '';

            // Simulate response
            setTimeout(() => {
                this.addToTerminal(`RX: ECHO: ${command}`);
            }, 100);
        }
    }

    addToTerminal(message) {
        const terminal = document.getElementById('uartTerminal');
        const timestamp = new Date().toLocaleTimeString();
        terminal.innerHTML += `<div>[${timestamp}] ${message}</div>`;
        terminal.scrollTop = terminal.scrollHeight;
    }

    sendProtocolCommand() {
        const input = document.getElementById('protocolCommand');
        const command = input.value.trim();
        const protocol = document.getElementById('protocolType').value;

        if (command) {
            this.logProtocol(`[${protocol.toUpperCase()}] TX: ${command}`);
            input.value = '';

            // Simulate protocol response
            setTimeout(() => {
                const response = this.generateProtocolResponse(protocol, command);
                this.logProtocol(`[${protocol.toUpperCase()}] RX: ${response}`);
                document.getElementById('protocolResponse').textContent = response;
            }, 200);
        }
    }

    generateProtocolResponse(protocol, command) {
        switch (protocol) {
            case 'dlms':
                if (command.startsWith('GET')) {
                    return 'DLMS Response: 1.0.1.8.0.255 = 12345.678*kWh';
                }
                return 'DLMS Response: OK';

            case 'modbus-rtu':
                return '01 03 02 0C 35 A1';

            case 'iec62056':
                if (command === '/?!') {
                    return '/SMT5\\2@1234567890';
                }
                return 'IEC Response: OK';

            default:
                return 'OK';
        }
    }

    logProtocol(message) {
        const log = document.getElementById('protocolLog');
        const timestamp = new Date().toLocaleTimeString();
        log.innerHTML += `<div>[${timestamp}] ${message}</div>`;
        log.scrollTop = log.scrollHeight;
    }

    logMessage(message, type = 'info') {
        const log = document.getElementById('systemLog');
        const timestamp = new Date().toLocaleTimeString();
        const prefix = type === 'error' ? '[ERROR]' : type === 'warning' ? '[WARN]' : '[INFO]';
        log.innerHTML += `<div>[${timestamp}] ${prefix} ${message}</div>`;
        log.scrollTop = log.scrollHeight;
    }

    exportLog() {
        const log = document.getElementById('systemLog');
        const content = log.textContent;
        const blob = new Blob([content], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);

        const a = document.createElement('a');
        a.href = url;
        a.download = `simulation_log_${new Date().toISOString().split('T')[0]}.txt`;
        a.click();

        URL.revokeObjectURL(url);
    }

    clearLog() {
        document.getElementById('systemLog').innerHTML = '';
    }

    updateProgress(value) {
        const fill = document.querySelector('.progress-fill');
        fill.style.width = `${value}%`;
    }

    initializeCharts() {
        this.setupWaveformChart();
        this.setupOscilloscopeChart();
        this.setupPerformanceChart();
        this.setupHarmonicsChart();
        this.setupPhasorChart();
    }

    setupWaveformChart() {
        const ctx = document.getElementById('waveformChart').getContext('2d');
        this.waveformChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: Array.from({length: 100}, (_, i) => i),
                datasets: [{
                    label: 'Voltage',
                    data: [],
                    borderColor: 'rgb(54, 162, 235)',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    tension: 0.4
                }, {
                    label: 'Current',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.4
                }]
            },
            options: {
                responsive: true,
                animation: false,
                scales: {
                    y: {
                        beginAtZero: true
                    }
                }
            }
        });
    }

    setupOscilloscopeChart() {
        const ctx = document.getElementById('oscilloscopeChart').getContext('2d');
        this.oscilloscopeChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: Array.from({length: 200}, (_, i) => i),
                datasets: [{
                    label: 'Channel 1',
                    data: [],
                    borderColor: 'rgb(255, 255, 0)',
                    backgroundColor: 'rgba(255, 255, 0, 0.1)',
                    tension: 0
                }, {
                    label: 'Channel 2',
                    data: [],
                    borderColor: 'rgb(0, 255, 255)',
                    backgroundColor: 'rgba(0, 255, 255, 0.1)',
                    tension: 0
                }]
            },
            options: {
                responsive: true,
                animation: false,
                plugins: {
                    legend: {
                        labels: {
                            color: 'white'
                        }
                    }
                },
                scales: {
                    x: {
                        grid: {
                            color: 'rgba(255, 255, 255, 0.3)'
                        },
                        ticks: {
                            color: 'white'
                        }
                    },
                    y: {
                        grid: {
                            color: 'rgba(255, 255, 255, 0.3)'
                        },
                        ticks: {
                            color: 'white'
                        }
                    }
                }
            }
        });
    }

    setupPerformanceChart() {
        const ctx = document.getElementById('performanceChart').getContext('2d');
        this.performanceChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'CPU Usage (%)',
                    data: [],
                    borderColor: 'rgb(75, 192, 192)',
                    backgroundColor: 'rgba(75, 192, 192, 0.1)',
                    tension: 0.4
                }, {
                    label: 'Memory Usage (%)',
                    data: [],
                    borderColor: 'rgb(255, 159, 64)',
                    backgroundColor: 'rgba(255, 159, 64, 0.1)',
                    tension: 0.4
                }, {
                    label: 'Active Power (W)',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.4
                }]
            },
            options: {
                responsive: true,
                animation: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100 // Assuming CPU/Memory are %
                    }
                }
            }
        });
    }

    setupHarmonicsChart() {
        const ctx = document.getElementById('harmonicsChart').getContext('2d');
        this.harmonicsChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: [],
                datasets: [{
                    label: 'Voltage Harmonics (%)',
                    data: [],
                    backgroundColor: 'rgba(54, 162, 235, 0.6)',
                    borderColor: 'rgba(54, 162, 235, 1)',
                    borderWidth: 1
                }, {
                    label: 'Current Harmonics (%)',
                    data: [],
                    backgroundColor: 'rgba(255, 99, 132, 0.6)',
                    borderColor: 'rgba(255, 99, 132, 1)',
                    borderWidth: 1
                }]
            },
            options: {
                responsive: true,
                animation: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        title: {
                            display: true,
                            text: 'Magnitude (%)'
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Harmonic Order'
                        }
                    }
                }
            }
        });
    }

    setupPhasorChart() {
        const ctx = document.getElementById('phasorChart').getContext('2d');
        this.phasorChart = new Chart(ctx, {
            type: 'scatter',
            data: {
                datasets: [{
                    label: 'Voltage Phasor',
                    data: [{ x: this.simulationData.phasors.voltage.real, y: this.simulationData.phasors.voltage.imag }],
                    backgroundColor: 'rgba(54, 162, 235, 0.8)',
                    borderColor: 'rgba(54, 162, 235, 1)',
                    pointRadius: 5,
                    showLine: false
                }, {
                    label: 'Current Phasor',
                    data: [{ x: this.simulationData.phasors.current.real, y: this.simulationData.phasors.current.imag }],
                    backgroundColor: 'rgba(255, 99, 132, 0.8)',
                    borderColor: 'rgba(255, 99, 132, 1)',
                    pointRadius: 5,
                    showLine: false
                }]
            },
            options: {
                responsive: true,
                animation: false,
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Real Part'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.3)'
                        },
                        ticks: {
                            color: 'white'
                        }
                    },
                    y: {
                        title: {
                            display: true,
                            text: 'Imaginary Part'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.3)'
                        },
                        ticks: {
                            color: 'white'
                        }
                    }
                },
                plugins: {
                    legend: {
                        labels: {
                            color: 'white'
                        }
                    }
                }
            }
        });
    }

    setupCircuitDesign() {
        // Initialize Fabric.js canvas for circuit design
        this.circuitCanvas = new fabric.Canvas('circuitCanvas');
        this.circuitCanvas.setBackgroundColor('#f8f9fa', this.circuitCanvas.renderAll.bind(this.circuitCanvas));

        // Add grid
        this.addGrid();

        // Setup component drag and drop
        this.setupComponentLibrary();
    }

    addGrid() {
        const gridSize = 20;
        const canvas = this.circuitCanvas;
        const width = canvas.getWidth();
        const height = canvas.getHeight();

        // Vertical lines
        for (let i = 0; i <= width; i += gridSize) {
            canvas.add(new fabric.Line([i, 0, i, height], {
                stroke: '#e0e0e0',
                strokeWidth: 1,
                selectable: false,
                evented: false
            }));
        }

        // Horizontal lines
        for (let i = 0; i <= height; i += gridSize) {
            canvas.add(new fabric.Line([0, i, width, i], {
                stroke: '#e0e0e0',
                strokeWidth: 1,
                selectable: false,
                evented: false
            }));
        }
    }

    setupComponentLibrary() {
        const componentItems = document.querySelectorAll('.component-item');
        componentItems.forEach(item => {
            item.addEventListener('click', () => {
                const componentType = item.dataset.type;
                this.addComponentToCanvas(componentType);
            });
        });
    }

    addComponentToCanvas(type) {
        let component;

        switch (type) {
            case 'resistor':
                component = new fabric.Rect({
                    left: 100,
                    top: 100,
                    width: 60,
                    height: 20,
                    fill: 'white',
                    stroke: 'black',
                    strokeWidth: 2
                });
                break;

            case 'capacitor':
                component = new fabric.Group([
                    new fabric.Line([0, 0, 0, 40], { stroke: 'black', strokeWidth: 3 }),
                    new fabric.Line([10, 0, 10, 40], { stroke: 'black', strokeWidth: 3 })
                ], {
                    left: 100,
                    top: 100
                });
                break;

            case 'led':
                component = new fabric.Circle({
                    left: 100,
                    top: 100,
                    radius: 15,
                    fill: 'red',
                    stroke: 'darkred',
                    strokeWidth: 2
                });
                break;

            default:
                component = new fabric.Rect({
                    left: 100,
                    top: 100,
                    width: 40,
                    height: 40,
                    fill: 'lightgray',
                    stroke: 'black',
                    strokeWidth: 1
                });
        }

        if (component) {
            this.circuitCanvas.add(component);
            this.circuitCanvas.setActiveObject(component);
        }
    }

    startUpdateLoop() {
        setInterval(() => {
            if (this.isRunning) {
                this.updateSimulation();
                this.updateCharts();
                this.updatePeripherals();
            }
        }, 100);
    }

    updateSimulation() {
        // Calculate measurements based on input values
        const voltage = this.simulationData.voltage;
        const current = this.simulationData.current;
        const powerFactor = this.simulationData.powerFactor;
        const frequency = this.simulationData.frequency;

        this.simulationData.measurements.voltageRMS = voltage;
        this.simulationData.measurements.currentRMS = current;
        this.simulationData.measurements.activePower = voltage * current * powerFactor;
        this.simulationData.measurements.reactivePower = voltage * current * Math.sin(Math.acos(powerFactor));
        this.simulationData.measurements.apparentPower = voltage * current;
        this.simulationData.measurements.powerFactor = powerFactor;
        this.simulationData.measurements.frequency = frequency;

        // Update harmonics (example: fundamental + some noise)
        this.simulationData.harmonics.voltage = this.generateHarmonics(voltage, frequency, 0.05); // 5% THD
        this.simulationData.harmonics.current = this.generateHarmonics(current, frequency, 0.1); // 10% THD

        // Update phasors (RMS values as magnitude, assume 0 phase for simplicity)
        this.simulationData.phasors.voltage = { real: voltage, imag: 0 };
        this.simulationData.phasors.current = { real: current * powerFactor, imag: current * Math.sin(Math.acos(powerFactor)) };


        // Update display
        this.updateMeasurementsDisplay();

        // Update progress
        const progress = (Date.now() % 10000) / 100;
        this.updateProgress(progress);
    }

    generateHarmonics(fundamental, frequency, thd) {
        const harmonics = [];
        const numHarmonics = 10; // Calculate up to 10th harmonic
        const fundamentalRad = 2 * Math.PI * frequency;

        for (let i = 1; i <= numHarmonics; i++) {
            const harmonicOrder = i;
            let magnitude = 0;
            if (harmonicOrder === 1) {
                magnitude = fundamental;
            } else {
                // Add random harmonic distortion
                magnitude = fundamental * thd * (Math.random() - 0.5) * 2;
            }
            harmonics.push({ order: harmonicOrder, magnitude: Math.max(0, magnitude) }); // Ensure magnitude is non-negative
        }
        return harmonics;
    }

    updateMeasurementsDisplay() {
        const measurements = this.simulationData.measurements;

        document.getElementById('voltageRMS').textContent = `${measurements.voltageRMS.toFixed(2)} V`;
        document.getElementById('currentRMS').textContent = `${measurements.currentRMS.toFixed(3)} A`;
        document.getElementById('activePower').textContent = `${measurements.activePower.toFixed(2)} W`;
        document.getElementById('reactivePower').textContent = `${measurements.reactivePower.toFixed(2)} VAR`;
        document.getElementById('apparentPower').textContent = `${measurements.apparentPower.toFixed(2)} VA`;
        document.getElementById('powerFactor').textContent = measurements.powerFactor.toFixed(3);
        document.getElementById('frequency').textContent = `${measurements.frequency.toFixed(2)} Hz`;
        document.getElementById('energy').textContent = `${(measurements.activePower * 0.001).toFixed(3)} kWh`;
    }

    initializeTabContent(tabName) {
        switch(tabName) {
            case 'harmonics':
                this.updateHarmonicsDisplay();
                break;
            case 'phasors':
                this.updatePhasorDisplay();
                break;
            case 'measurements':
                this.updateOscilloscopeDisplay();
                break;
            case 'circuit':
                this.refreshCircuitCanvas();
                break;
        }
    }

    updateCharts() {
        // Update waveform data
        this.updateWaveformChart();
        
        // Update performance chart with new data
        if (this.performanceChart && this.isRunning) {
            const now = Date.now();
            this.performanceChart.data.labels.push(new Date(now).toLocaleTimeString());
            this.performanceChart.data.datasets[0].data.push(Math.random() * 100); // CPU usage
            this.performanceChart.data.datasets[1].data.push(Math.random() * 80);  // Memory usage
            this.performanceChart.data.datasets[2].data.push(this.simulationData.measurements.activePower);

            // Keep only last 20 data points
            if (this.performanceChart.data.labels.length > 20) {
                this.performanceChart.data.labels.shift();
                this.performanceChart.data.datasets.forEach(dataset => dataset.data.shift());
            }

            this.performanceChart.update('none');
        }

        // Update tab-specific charts based on current active tab
        if (this.currentTab === 'harmonics') {
            this.updateHarmonicsDisplay();
        } else if (this.currentTab === 'phasors') {
            this.updatePhasorDisplay();
        }
    }

    updateWaveformChart() {
        if (!this.waveformChart || !this.isRunning) return;
        
        const time = Date.now() / 1000;
        const frequency = this.simulationData.frequency;
        const voltageAmplitude = this.simulationData.voltage * Math.sqrt(2);
        const currentAmplitude = this.simulationData.current * Math.sqrt(2);
        const powerFactor = this.simulationData.powerFactor;
        
        const voltageData = [];
        const currentData = [];
        
        for (let i = 0; i < 100; i++) {
            const t = (time + i * 0.001) * 2 * Math.PI * frequency;
            voltageData.push(voltageAmplitude * Math.sin(t));
            currentData.push(currentAmplitude * Math.sin(t - Math.acos(powerFactor)));
        }
        
        this.waveformChart.data.datasets[0].data = voltageData;
        this.waveformChart.data.datasets[1].data = currentData;
        this.waveformChart.update('none');
    }

    updateHarmonicsDisplay() {
        if (!this.harmonicsChart) return;
        
        // Generate up to 33rd harmonic
        const harmonicLabels = [];
        const voltageHarmonics = [];
        const currentHarmonics = [];
        
        for (let h = 1; h <= 33; h++) {
            harmonicLabels.push(h.toString());
            
            if (h === 1) {
                voltageHarmonics.push(100); // Fundamental is 100%
                currentHarmonics.push(100);
            } else if (h % 2 === 1 && h <= 13) { // Odd harmonics up to 13th
                voltageHarmonics.push(Math.random() * 8 + 1);
                currentHarmonics.push(Math.random() * 12 + 2);
            } else if (h <= 25) {
                voltageHarmonics.push(Math.random() * 3 + 0.5);
                currentHarmonics.push(Math.random() * 5 + 1);
            } else {
                voltageHarmonics.push(Math.random() * 1 + 0.1);
                currentHarmonics.push(Math.random() * 2 + 0.2);
            }
        }

        this.harmonicsChart.data.labels = harmonicLabels;
        this.harmonicsChart.data.datasets[0].data = voltageHarmonics;
        this.harmonicsChart.data.datasets[1].data = currentHarmonics;
        this.harmonicsChart.update();
        
        // Update harmonics table
        this.updateHarmonicsTable(harmonicLabels, voltageHarmonics, currentHarmonics);
    }

    updateHarmonicsTable(labels, voltageHarmonics, currentHarmonics) {
        const tableBody = document.getElementById('harmonicsTableBody');
        if (!tableBody) return;
        
        tableBody.innerHTML = '';
        
        for (let i = 0; i < Math.min(labels.length, 33); i++) {
            const row = document.createElement('tr');
            const frequency = this.simulationData.frequency * parseInt(labels[i]);
            const phase = Math.random() * 360 - 180; // Random phase
            
            row.innerHTML = `
                <td>${labels[i]}</td>
                <td>${frequency.toFixed(1)}</td>
                <td>${voltageHarmonics[i].toFixed(2)}</td>
                <td>${currentHarmonics[i].toFixed(2)}</td>
                <td>${phase.toFixed(1)}</td>
            `;
            tableBody.appendChild(row);
        }
    }

    updatePhasorDisplay() {
        if (!this.phasorChart) return;
        
        // Calculate three-phase phasors
        const voltageMag = this.simulationData.voltage;
        const currentMag = this.simulationData.current;
        const powerFactorAngle = Math.acos(this.simulationData.powerFactor) * 180 / Math.PI;
        
        const voltagePhasors = [];
        const currentPhasors = [];
        
        // Three phases: L1, L2, L3
        for (let phase = 0; phase < 3; phase++) {
            const voltageAngle = phase * 120; // 120 degree separation
            const currentAngle = voltageAngle - powerFactorAngle; // Current lags voltage
            
            // Convert to Cartesian coordinates
            const vReal = voltageMag * Math.cos(voltageAngle * Math.PI / 180);
            const vImag = voltageMag * Math.sin(voltageAngle * Math.PI / 180);
            const iReal = currentMag * Math.cos(currentAngle * Math.PI / 180);
            const iImag = currentMag * Math.sin(currentAngle * Math.PI / 180);
            
            voltagePhasors.push({ x: vReal, y: vImag });
            currentPhasors.push({ x: iReal, y: iImag });
        }

        this.phasorChart.data.datasets[0].data = voltagePhasors;
        this.phasorChart.data.datasets[1].data = currentPhasors;
        this.phasorChart.update();
        
        // Update phasor tables
        this.updatePhasorTables(voltagePhasors, currentPhasors);
    }

    updatePhasorTables(voltagePhasors, currentPhasors) {
        const voltageTable = document.getElementById('voltagePhasorTable');
        const currentTable = document.getElementById('currentPhasorTable');
        
        if (voltageTable) {
            voltageTable.innerHTML = '';
            voltagePhasors.forEach((phasor, index) => {
                const magnitude = Math.sqrt(phasor.x * phasor.x + phasor.y * phasor.y);
                const angle = Math.atan2(phasor.y, phasor.x) * 180 / Math.PI;
                
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td>L${index + 1}</td>
                    <td>${magnitude.toFixed(2)} V</td>
                    <td>${angle.toFixed(1)}°</td>
                `;
                voltageTable.appendChild(row);
            });
        }
        
        if (currentTable) {
            currentTable.innerHTML = '';
            currentPhasors.forEach((phasor, index) => {
                const magnitude = Math.sqrt(phasor.x * phasor.x + phasor.y * phasor.y);
                const angle = Math.atan2(phasor.y, phasor.x) * 180 / Math.PI;
                
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td>L${index + 1}</td>
                    <td>${magnitude.toFixed(2)} A</td>
                    <td>${angle.toFixed(1)}°</td>
                `;
                currentTable.appendChild(row);
            });
        }
    }

    updateOscilloscopeDisplay() {
        if (!this.oscilloscopeChart || !this.isRunning) return;
        
        const time = Date.now() / 1000;
        const frequency = this.simulationData.frequency;
        const voltageAmplitude = this.simulationData.voltage * Math.sqrt(2);
        const currentAmplitude = this.simulationData.current * Math.sqrt(2);
        
        const ch1Data = [];
        const ch2Data = [];
        
        for (let i = 0; i < 200; i++) {
            const t = (time + i * 0.0005) * 2 * Math.PI * frequency;
            ch1Data.push(voltageAmplitude * Math.sin(t) + Math.random() * 5 - 2.5); // Add noise
            ch2Data.push(currentAmplitude * 10 * Math.sin(t - Math.PI/4) + Math.random() * 2 - 1); // Scale current for visibility
        }
        
        this.oscilloscopeChart.data.datasets[0].data = ch1Data;
        this.oscilloscopeChart.data.datasets[1].data = ch2Data;
        this.oscilloscopeChart.update('none');
    }

    refreshCircuitCanvas() {
        if (this.circuitCanvas) {
            this.circuitCanvas.renderAll();
        }
    }

    updatePeripherals() {
        this.updateGPIOTable();
        this.updateADCTable();
        this.updateTimerTable();
        this.updateMultimeter();
    }

    updateGPIOTable() {
        const gpioContainer = document.getElementById('gpioTable');
        if (!gpioContainer.hasChildNodes()) {
            for (let i = 0; i < 16; i++) {
                const item = document.createElement('div');
                item.className = 'gpio-item';
                item.innerHTML = `
                    <span>GPIO${i}</span>
                    <span id="gpio${i}State">${Math.random() > 0.5 ? 'HIGH' : 'LOW'}</span>
                `;
                gpioContainer.appendChild(item);
            }
        } else {
            // Update GPIO states
            for (let i = 0; i < 16; i++) {
                const stateElement = document.getElementById(`gpio${i}State`);
                if (stateElement) {
                    stateElement.textContent = Math.random() > 0.5 ? 'HIGH' : 'LOW';
                }
            }
        }
    }

    updateADCTable() {
        const adcContainer = document.getElementById('adcTable');
        if (!adcContainer.hasChildNodes()) {
            for (let i = 0; i < 8; i++) {
                const item = document.createElement('div');
                item.className = 'adc-item';
                item.innerHTML = `
                    <span>ADC${i}</span>
                    <span id="adc${i}Value">${(Math.random() * 3.3).toFixed(3)}V</span>
                `;
                adcContainer.appendChild(item);
            }
        } else {
            // Update ADC values
            for (let i = 0; i < 8; i++) {
                const valueElement = document.getElementById(`adc${i}Value`);
                if (valueElement) {
                    valueElement.textContent = `${(Math.random() * 3.3).toFixed(3)}V`;
                }
            }
        }
    }

    updateTimerTable() {
        const timerContainer = document.getElementById('timerTable');
        if (!timerContainer.hasChildNodes()) {
            for (let i = 0; i < 4; i++) {
                const item = document.createElement('div');
                item.className = 'timer-item';
                item.innerHTML = `
                    <span>Timer${i}</span>
                    <span id="timer${i}Status">${Math.random() > 0.5 ? 'Running' : 'Stopped'}</span>
                `;
                timerContainer.appendChild(item);
            }
        } else {
            // Update timer status
            for (let i = 0; i < 4; i++) {
                const statusElement = document.getElementById(`timer${i}Status`);
                if (statusElement) {
                    statusElement.textContent = Math.random() > 0.5 ? 'Running' : 'Stopped';
                }
            }
        }
    }

    updateMultimeter() {
        const mode = document.getElementById('multimeterMode').value;
        const reading = document.getElementById('multimeterReading');
        const unit = document.getElementById('multimeterUnit');

        let value, unitText;

        switch (mode) {
            case 'dcv':
            case 'acv':
                value = this.simulationData.measurements.voltageRMS;
                unitText = 'V';
                break;
            case 'dca':
            case 'aca':
                value = this.simulationData.measurements.currentRMS;
                unitText = 'A';
                break;
            case 'resistance':
                value = Math.random() * 1000;
                unitText = 'Ω';
                break;
            case 'frequency':
                value = this.simulationData.measurements.frequency;
                unitText = 'Hz';
                break;
            default:
                value = 0;
                unitText = '';
        }

        reading.textContent = value.toFixed(3);
        unit.textContent = unitText;
    }

    loadMCUPartNumbers() {
        const familySelect = document.getElementById('mcuFamily');
        const partNumberSelect = document.getElementById('mcuPartNumber');
        const selectedFamily = familySelect.value;

        // Clear existing part numbers
        partNumberSelect.innerHTML = '<option value="">Select Part Number</option>';

        const mcuData = {
            'STM32': ['STM32F103', 'STM32F407', 'STM32L476'],
            'ESP32': ['ESP32-WROOM-32', 'ESP32-S3', 'ESP32-C3'],
            'AVR': ['ATmega328P', 'ATtiny85', 'ATmega2560']
        };

        if (mcuData[selectedFamily]) {
            mcuData[selectedFamily].forEach(partNumber => {
                const option = document.createElement('option');
                option.value = partNumber;
                option.textContent = partNumber;
                partNumberSelect.appendChild(option);
            });
        }
        this.updateMCUConfiguration(); // Update configuration even if no part number is selected
    }

    updateMCUConfiguration() {
        const family = document.getElementById('mcuFamily').value;
        const partNumber = document.getElementById('mcuPartNumber').value;
        const architecture = document.getElementById('mcuArchitecture').value;

        // In a real application, you would use this information to configure the simulation
        // For now, we'll just log it.
        this.logMessage(`MCU Configuration Updated: Family=${family}, Part Number=${partNumber}, Architecture=${architecture}`, 'info');
    }
}

// Initialize application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.simulator = new SmartMeterSimulator();
});