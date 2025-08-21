
// Chart utilities and configurations
class ChartManager {
    constructor() {
        this.charts = {};
        this.colors = {
            voltage: 'rgb(54, 162, 235)',
            current: 'rgb(255, 99, 132)',
            power: 'rgb(75, 192, 192)',
            frequency: 'rgb(255, 205, 86)'
        };
    }

    createWaveformChart(canvasId) {
        const ctx = document.getElementById(canvasId).getContext('2d');
        const chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: Array.from({length: 100}, (_, i) => i),
                datasets: [{
                    label: 'Voltage (V)',
                    data: [],
                    borderColor: this.colors.voltage,
                    backgroundColor: this.colors.voltage + '20',
                    tension: 0.4,
                    pointRadius: 0
                }, {
                    label: 'Current (A)',
                    data: [],
                    borderColor: this.colors.current,
                    backgroundColor: this.colors.current + '20',
                    tension: 0.4,
                    pointRadius: 0
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    x: {
                        display: true,
                        title: {
                            display: true,
                            text: 'Time (ms)'
                        }
                    },
                    y: {
                        display: true,
                        title: {
                            display: true,
                            text: 'Amplitude'
                        }
                    }
                },
                plugins: {
                    legend: {
                        display: true,
                        position: 'top'
                    }
                }
            }
        });
        
        this.charts[canvasId] = chart;
        return chart;
    }

    createOscilloscopeChart(canvasId) {
        const ctx = document.getElementById(canvasId).getContext('2d');
        const chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: Array.from({length: 200}, (_, i) => i),
                datasets: [{
                    label: 'Channel 1',
                    data: [],
                    borderColor: '#ffff00',
                    backgroundColor: 'rgba(255, 255, 0, 0.1)',
                    tension: 0,
                    pointRadius: 0
                }, {
                    label: 'Channel 2',
                    data: [],
                    borderColor: '#00ffff',
                    backgroundColor: 'rgba(0, 255, 255, 0.1)',
                    tension: 0,
                    pointRadius: 0
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                backgroundColor: '#000000',
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
        
        this.charts[canvasId] = chart;
        return chart;
    }

    createLogicAnalyzerChart(canvasId) {
        const ctx = document.getElementById(canvasId).getContext('2d');
        const chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: Array.from({length: 100}, (_, i) => i),
                datasets: [{
                    label: 'Channel 0',
                    data: [],
                    borderColor: '#ff0000',
                    backgroundColor: '#ff0000',
                    stepped: true,
                    pointRadius: 0
                }, {
                    label: 'Channel 1',
                    data: [],
                    borderColor: '#00ff00',
                    backgroundColor: '#00ff00',
                    stepped: true,
                    pointRadius: 0
                }, {
                    label: 'Channel 2',
                    data: [],
                    borderColor: '#0000ff',
                    backgroundColor: '#0000ff',
                    stepped: true,
                    pointRadius: 0
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Time (μs)'
                        }
                    },
                    y: {
                        min: -0.5,
                        max: 3.5,
                        ticks: {
                            stepSize: 1,
                            callback: function(value) {
                                return value >= 0 && value <= 2 ? `CH${value}` : '';
                            }
                        }
                    }
                }
            }
        });
        
        this.charts[canvasId] = chart;
        return chart;
    }

    createPerformanceChart(canvasId) {
        const ctx = document.getElementById(canvasId).getContext('2d');
        const chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'CPU Usage (%)',
                    data: [],
                    borderColor: this.colors.power,
                    backgroundColor: this.colors.power + '20',
                    tension: 0.4
                }, {
                    label: 'Memory Usage (%)',
                    data: [],
                    borderColor: this.colors.frequency,
                    backgroundColor: this.colors.frequency + '20',
                    tension: 0.4
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100,
                        title: {
                            display: true,
                            text: 'Usage (%)'
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Time'
                        }
                    }
                }
            }
        });
        
        this.charts[canvasId] = chart;
        return chart;
    }

    updateChart(chartId, datasetIndex, newData) {
        const chart = this.charts[chartId];
        if (chart && chart.data.datasets[datasetIndex]) {
            chart.data.datasets[datasetIndex].data = newData;
            chart.update('none');
        }
    }

    addDataPoint(chartId, datasetIndex, value) {
        const chart = this.charts[chartId];
        if (chart && chart.data.datasets[datasetIndex]) {
            const data = chart.data.datasets[datasetIndex].data;
            data.push(value);
            
            // Keep only last 100 points
            if (data.length > 100) {
                data.shift();
            }
            
            chart.update('none');
        }
    }

    generateSineWave(amplitude, frequency, sampleRate, samples, phase = 0) {
        const data = [];
        for (let i = 0; i < samples; i++) {
            const t = i / sampleRate;
            const value = amplitude * Math.sin(2 * Math.PI * frequency * t + phase);
            data.push(value);
        }
        return data;
    }

    generateSquareWave(amplitude, frequency, sampleRate, samples) {
        const data = [];
        for (let i = 0; i < samples; i++) {
            const t = i / sampleRate;
            const value = amplitude * Math.sign(Math.sin(2 * Math.PI * frequency * t));
            data.push(value);
        }
        return data;
    }

    generateDigitalSignal(channels, samples) {
        const data = [];
        for (let channel = 0; channel < channels; channel++) {
            const channelData = [];
            for (let i = 0; i < samples; i++) {
                // Generate random digital signal
                const value = channel + (Math.random() > 0.5 ? 0.8 : 0.2);
                channelData.push(value);
            }
            data.push(channelData);
        }
        return data;
    }
}

// Global chart manager instance
window.chartManager = new ChartManager();
