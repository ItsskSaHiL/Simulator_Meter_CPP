
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QTimer>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <cmath>

#include "simulator_core.h"
#include "mcu_emulator.h"
#include "metering_engine.h"
#include "protocol_handler.h"

class SmartMeterSimulator : public QMainWindow
{
    Q_OBJECT

public:
    SmartMeterSimulator(QWidget *parent = nullptr);
    ~SmartMeterSimulator();

private slots:
    void loadFirmware();
    void startSimulation();
    void stopSimulation();
    void resetSimulation();
    void updateDisplay();
    void onProtocolCommand();
    void injectTamperEvent();
    void exportResults();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupFirmwareTab();
    void setupMeteringTab();
    void setupPeripheralsTab();
    void setupProtocolsTab();
    void setupLoggingTab();

    // Core components
    std::unique_ptr<SimulatorCore> m_core;
    std::unique_ptr<MCUEmulator> m_mcuEmulator;
    std::unique_ptr<MeteringEngine> m_meteringEngine;
    std::unique_ptr<ProtocolHandler> m_protocolHandler;

    // UI Components
    QTabWidget *m_tabWidget;
    QTimer *m_updateTimer;
    
    // Firmware Tab
    QWidget *m_firmwareTab;
    QLineEdit *m_firmwarePathEdit;
    QComboBox *m_mcuFamilyCombo;
    QComboBox *m_mcuPartCombo;
    QComboBox *m_packageCombo;
    QComboBox *m_architectureCombo;
    QPushButton *m_loadFirmwareBtn;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_resetBtn;
    QProgressBar *m_simulationProgress;
    QLabel *m_statusLabel;

    // Metering Tab
    QWidget *m_meteringTab;
    QComboBox *m_phaseConfigCombo;
    QDoubleSpinBox *m_voltageAmplitude;
    QDoubleSpinBox *m_currentAmplitude;
    QDoubleSpinBox *m_frequency;
    QDoubleSpinBox *m_powerFactor;
    QTableWidget *m_meteringDisplay;
    QGraphicsView *m_waveformView;
    QGraphicsScene *m_waveformScene;
    QPushButton *m_tamperBtn;
    QComboBox *m_tamperTypeCombo;

    // Peripherals Tab
    QWidget *m_peripheralsTab;
    QTextEdit *m_uartTerminal;
    QLineEdit *m_uartInput;
    QTableWidget *m_gpioTable;
    QTableWidget *m_adcTable;
    QTableWidget *m_timerTable;

    // Protocols Tab
    QWidget *m_protocolsTab;
    QComboBox *m_protocolCombo;
    QTextEdit *m_protocolLog;
    QLineEdit *m_protocolCommand;
    QPushButton *m_sendCommandBtn;
    QTextEdit *m_protocolResponse;

    // Logging Tab
    QWidget *m_loggingTab;
    QTextEdit *m_systemLog;
    QPushButton *m_exportBtn;
    QPushButton *m_clearLogBtn;

    bool m_simulationRunning;
};

SmartMeterSimulator::SmartMeterSimulator(QWidget *parent)
    : QMainWindow(parent)
    , m_simulationRunning(false)
{
    // Initialize core components
    m_core = std::make_unique<SimulatorCore>();
    m_mcuEmulator = std::make_unique<MCUEmulator>();
    m_meteringEngine = std::make_unique<MeteringEngine>();
    m_protocolHandler = std::make_unique<ProtocolHandler>();

    setupUI();
    setupMenuBar();
    setupStatusBar();

    // Setup update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &SmartMeterSimulator::updateDisplay);
    m_updateTimer->start(100); // Update every 100ms

    setWindowTitle("Smart Meter Firmware Simulator v1.0");
    resize(1200, 800);
}

SmartMeterSimulator::~SmartMeterSimulator() = default;

void SmartMeterSimulator::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    setupFirmwareTab();
    setupMeteringTab();
    setupPeripheralsTab();
    setupProtocolsTab();
    setupLoggingTab();
}

void SmartMeterSimulator::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction("Load Firmware", this, &SmartMeterSimulator::loadFirmware);
    fileMenu->addAction("Export Results", this, &SmartMeterSimulator::exportResults);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QWidget::close);

    QMenu *simulationMenu = menuBar->addMenu("&Simulation");
    simulationMenu->addAction("Start", this, &SmartMeterSimulator::startSimulation);
    simulationMenu->addAction("Stop", this, &SmartMeterSimulator::stopSimulation);
    simulationMenu->addAction("Reset", this, &SmartMeterSimulator::resetSimulation);

    QMenu *helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("About", [this]() {
        QMessageBox::about(this, "About", "Smart Meter Firmware Simulator\nVersion 1.0");
    });
}

void SmartMeterSimulator::setupStatusBar()
{
    QStatusBar *statusBar = this->statusBar();
    statusBar->showMessage("Ready");
}

void SmartMeterSimulator::setupFirmwareTab()
{
    m_firmwareTab = new QWidget();
    m_tabWidget->addTab(m_firmwareTab, "Firmware");

    QVBoxLayout *layout = new QVBoxLayout(m_firmwareTab);

    // Firmware Loading Group
    QGroupBox *firmwareGroup = new QGroupBox("Firmware Loading");
    QGridLayout *firmwareLayout = new QGridLayout(firmwareGroup);

    firmwareLayout->addWidget(new QLabel("Firmware File:"), 0, 0);
    m_firmwarePathEdit = new QLineEdit();
    m_firmwarePathEdit->setReadOnly(true);
    firmwareLayout->addWidget(m_firmwarePathEdit, 0, 1);
    
    QPushButton *browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, this, &SmartMeterSimulator::loadFirmware);
    firmwareLayout->addWidget(browseBtn, 0, 2);

    firmwareLayout->addWidget(new QLabel("MCU Family:"), 1, 0);
    m_mcuFamilyCombo = new QComboBox();
    m_mcuFamilyCombo->addItems({"STM32F4", "STM32F7", "STM32H7", "Renesas RL78", "TI MSP430", "NXP LPC"});
    firmwareLayout->addWidget(m_mcuFamilyCombo, 1, 1);

    firmwareLayout->addWidget(new QLabel("Part Number:"), 2, 0);
    m_mcuPartCombo = new QComboBox();
    m_mcuPartCombo->addItems({"STM32F407VG", "STM32F429ZI", "RL78/I1C", "MSP430F5529", "LPC1768"});
    firmwareLayout->addWidget(m_mcuPartCombo, 2, 1);

    firmwareLayout->addWidget(new QLabel("Package:"), 3, 0);
    m_packageCombo = new QComboBox();
    m_packageCombo->addItems({"100-pin LQFP", "144-pin LQFP", "176-pin LQFP", "64-pin QFN"});
    firmwareLayout->addWidget(m_packageCombo, 3, 1);

    firmwareLayout->addWidget(new QLabel("Architecture:"), 4, 0);
    m_architectureCombo = new QComboBox();
    m_architectureCombo->addItems({"ARM Cortex-M4", "ARM Cortex-M7", "RL78", "MSP430", "ARM Cortex-M3"});
    firmwareLayout->addWidget(m_architectureCombo, 4, 1);

    layout->addWidget(firmwareGroup);

    // Control Group
    QGroupBox *controlGroup = new QGroupBox("Simulation Control");
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    m_startBtn = new QPushButton("Start");
    connect(m_startBtn, &QPushButton::clicked, this, &SmartMeterSimulator::startSimulation);
    controlLayout->addWidget(m_startBtn);

    m_stopBtn = new QPushButton("Stop");
    m_stopBtn->setEnabled(false);
    connect(m_stopBtn, &QPushButton::clicked, this, &SmartMeterSimulator::stopSimulation);
    controlLayout->addWidget(m_stopBtn);

    m_resetBtn = new QPushButton("Reset");
    connect(m_resetBtn, &QPushButton::clicked, this, &SmartMeterSimulator::resetSimulation);
    controlLayout->addWidget(m_resetBtn);

    controlLayout->addStretch();

    layout->addWidget(controlGroup);

    // Status Group
    QGroupBox *statusGroup = new QGroupBox("Status");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);

    m_statusLabel = new QLabel("Stopped");
    statusLayout->addWidget(m_statusLabel);

    m_simulationProgress = new QProgressBar();
    m_simulationProgress->setRange(0, 100);
    statusLayout->addWidget(m_simulationProgress);

    layout->addWidget(statusGroup);

    layout->addStretch();
}

void SmartMeterSimulator::setupMeteringTab()
{
    m_meteringTab = new QWidget();
    m_tabWidget->addTab(m_meteringTab, "Metering");

    QHBoxLayout *layout = new QHBoxLayout(m_meteringTab);

    // Left panel - Configuration
    QVBoxLayout *leftLayout = new QVBoxLayout();

    QGroupBox *configGroup = new QGroupBox("AC Input Configuration");
    QGridLayout *configLayout = new QGridLayout(configGroup);

    configLayout->addWidget(new QLabel("Phase Config:"), 0, 0);
    m_phaseConfigCombo = new QComboBox();
    m_phaseConfigCombo->addItems({"Single Phase", "Three Phase"});
    configLayout->addWidget(m_phaseConfigCombo, 0, 1);

    configLayout->addWidget(new QLabel("Voltage (V):"), 1, 0);
    m_voltageAmplitude = new QDoubleSpinBox();
    m_voltageAmplitude->setRange(0, 500);
    m_voltageAmplitude->setValue(230);
    configLayout->addWidget(m_voltageAmplitude, 1, 1);

    configLayout->addWidget(new QLabel("Current (A):"), 2, 0);
    m_currentAmplitude = new QDoubleSpinBox();
    m_currentAmplitude->setRange(0, 100);
    m_currentAmplitude->setValue(5);
    configLayout->addWidget(m_currentAmplitude, 2, 1);

    configLayout->addWidget(new QLabel("Frequency (Hz):"), 3, 0);
    m_frequency = new QDoubleSpinBox();
    m_frequency->setRange(45, 65);
    m_frequency->setValue(50);
    configLayout->addWidget(m_frequency, 3, 1);

    configLayout->addWidget(new QLabel("Power Factor:"), 4, 0);
    m_powerFactor = new QDoubleSpinBox();
    m_powerFactor->setRange(0, 1);
    m_powerFactor->setValue(0.95);
    m_powerFactor->setSingleStep(0.01);
    configLayout->addWidget(m_powerFactor, 4, 1);

    leftLayout->addWidget(configGroup);

    // Tamper Events
    QGroupBox *tamperGroup = new QGroupBox("Tamper Events");
    QVBoxLayout *tamperLayout = new QVBoxLayout(tamperGroup);

    m_tamperTypeCombo = new QComboBox();
    m_tamperTypeCombo->addItems({"Magnet Tamper", "Reverse Current", "Neutral Missing", "Phase Loss", "Over Voltage"});
    tamperLayout->addWidget(m_tamperTypeCombo);

    m_tamperBtn = new QPushButton("Inject Tamper Event");
    connect(m_tamperBtn, &QPushButton::clicked, this, &SmartMeterSimulator::injectTamperEvent);
    tamperLayout->addWidget(m_tamperBtn);

    leftLayout->addWidget(tamperGroup);
    leftLayout->addStretch();

    layout->addLayout(leftLayout);

    // Right panel - Display
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // Metering Display
    QGroupBox *displayGroup = new QGroupBox("Real-time Measurements");
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);

    m_meteringDisplay = new QTableWidget(8, 2);
    m_meteringDisplay->setHorizontalHeaderLabels({"Parameter", "Value"});
    m_meteringDisplay->horizontalHeader()->setStretchLastSection(true);
    m_meteringDisplay->setItem(0, 0, new QTableWidgetItem("Voltage RMS"));
    m_meteringDisplay->setItem(1, 0, new QTableWidgetItem("Current RMS"));
    m_meteringDisplay->setItem(2, 0, new QTableWidgetItem("Active Power"));
    m_meteringDisplay->setItem(3, 0, new QTableWidgetItem("Reactive Power"));
    m_meteringDisplay->setItem(4, 0, new QTableWidgetItem("Apparent Power"));
    m_meteringDisplay->setItem(5, 0, new QTableWidgetItem("Power Factor"));
    m_meteringDisplay->setItem(6, 0, new QTableWidgetItem("Frequency"));
    m_meteringDisplay->setItem(7, 0, new QTableWidgetItem("Energy"));

    displayLayout->addWidget(m_meteringDisplay);
    rightLayout->addWidget(displayGroup);

    // Waveform Display
    QGroupBox *waveformGroup = new QGroupBox("Waveforms");
    QVBoxLayout *waveformLayout = new QVBoxLayout(waveformGroup);

    m_waveformView = new QGraphicsView();
    m_waveformScene = new QGraphicsScene();
    m_waveformView->setScene(m_waveformScene);
    m_waveformView->setMinimumHeight(200);
    waveformLayout->addWidget(m_waveformView);

    rightLayout->addWidget(waveformGroup);

    layout->addLayout(rightLayout);
}

void SmartMeterSimulator::setupPeripheralsTab()
{
    m_peripheralsTab = new QWidget();
    m_tabWidget->addTab(m_peripheralsTab, "Peripherals");

    QHBoxLayout *layout = new QHBoxLayout(m_peripheralsTab);

    // UART Terminal
    QGroupBox *uartGroup = new QGroupBox("UART Terminal");
    QVBoxLayout *uartLayout = new QVBoxLayout(uartGroup);

    m_uartTerminal = new QTextEdit();
    m_uartTerminal->setReadOnly(true);
    m_uartTerminal->setMaximumHeight(150);
    uartLayout->addWidget(m_uartTerminal);

    QHBoxLayout *uartInputLayout = new QHBoxLayout();
    m_uartInput = new QLineEdit();
    uartInputLayout->addWidget(m_uartInput);

    QPushButton *sendBtn = new QPushButton("Send");
    connect(sendBtn, &QPushButton::clicked, [this]() {
        QString text = m_uartInput->text();
        m_uartTerminal->append("TX: " + text);
        m_uartInput->clear();
        // Send to UART emulation
    });
    uartInputLayout->addWidget(sendBtn);

    uartLayout->addLayout(uartInputLayout);
    layout->addWidget(uartGroup);

    // GPIO Table
    QGroupBox *gpioGroup = new QGroupBox("GPIO Status");
    QVBoxLayout *gpioLayout = new QVBoxLayout(gpioGroup);

    m_gpioTable = new QTableWidget(16, 3);
    m_gpioTable->setHorizontalHeaderLabels({"Pin", "Direction", "State"});
    m_gpioTable->horizontalHeader()->setStretchLastSection(true);
    gpioLayout->addWidget(m_gpioTable);

    layout->addWidget(gpioGroup);

    // ADC Values
    QGroupBox *adcGroup = new QGroupBox("ADC Channels");
    QVBoxLayout *adcLayout = new QVBoxLayout(gpioGroup);

    m_adcTable = new QTableWidget(8, 2);
    m_adcTable->setHorizontalHeaderLabels({"Channel", "Value"});
    m_adcTable->horizontalHeader()->setStretchLastSection(true);
    adcLayout->addWidget(m_adcTable);

    layout->addWidget(adcGroup);
}

void SmartMeterSimulator::setupProtocolsTab()
{
    m_protocolsTab = new QWidget();
    m_tabWidget->addTab(m_protocolsTab, "Protocols");

    QVBoxLayout *layout = new QVBoxLayout(m_protocolsTab);

    QGroupBox *protocolGroup = new QGroupBox("Protocol Interface");
    QVBoxLayout *protocolLayout = new QVBoxLayout(protocolGroup);

    QHBoxLayout *selectLayout = new QHBoxLayout();
    selectLayout->addWidget(new QLabel("Protocol:"));
    m_protocolCombo = new QComboBox();
    m_protocolCombo->addItems({"DLMS/COSEM", "Modbus RTU", "Modbus TCP", "IEC 62056", "Custom"});
    selectLayout->addWidget(m_protocolCombo);
    selectLayout->addStretch();

    protocolLayout->addLayout(selectLayout);

    // Command Input
    QHBoxLayout *commandLayout = new QHBoxLayout();
    commandLayout->addWidget(new QLabel("Command:"));
    m_protocolCommand = new QLineEdit();
    commandLayout->addWidget(m_protocolCommand);

    m_sendCommandBtn = new QPushButton("Send");
    connect(m_sendCommandBtn, &QPushButton::clicked, this, &SmartMeterSimulator::onProtocolCommand);
    commandLayout->addWidget(m_sendCommandBtn);

    protocolLayout->addLayout(commandLayout);

    // Response Display
    protocolLayout->addWidget(new QLabel("Response:"));
    m_protocolResponse = new QTextEdit();
    m_protocolResponse->setMaximumHeight(100);
    protocolLayout->addWidget(m_protocolResponse);

    layout->addWidget(protocolGroup);

    // Protocol Log
    QGroupBox *logGroup = new QGroupBox("Protocol Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);

    m_protocolLog = new QTextEdit();
    m_protocolLog->setReadOnly(true);
    logLayout->addWidget(m_protocolLog);

    layout->addWidget(logGroup);
}

void SmartMeterSimulator::setupLoggingTab()
{
    m_loggingTab = new QWidget();
    m_tabWidget->addTab(m_loggingTab, "Logging");

    QVBoxLayout *layout = new QVBoxLayout(m_loggingTab);

    QGroupBox *logGroup = new QGroupBox("System Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);

    m_systemLog = new QTextEdit();
    m_systemLog->setReadOnly(true);
    logLayout->addWidget(m_systemLog);

    QHBoxLayout *logControlLayout = new QHBoxLayout();
    
    m_exportBtn = new QPushButton("Export to CSV");
    connect(m_exportBtn, &QPushButton::clicked, this, &SmartMeterSimulator::exportResults);
    logControlLayout->addWidget(m_exportBtn);

    m_clearLogBtn = new QPushButton("Clear Log");
    connect(m_clearLogBtn, &QPushButton::clicked, [this]() {
        m_systemLog->clear();
    });
    logControlLayout->addWidget(m_clearLogBtn);

    logControlLayout->addStretch();
    logLayout->addLayout(logControlLayout);

    layout->addWidget(logGroup);
}

void SmartMeterSimulator::loadFirmware()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Load Firmware", "", "Firmware Files (*.hex *.bin)");
    
    if (!fileName.isEmpty()) {
        m_firmwarePathEdit->setText(fileName);
        m_systemLog->append(QString("Firmware loaded: %1").arg(fileName));
        
        // Load firmware into emulator
        std::string firmwareFile = fileName.toStdString();
        if (m_mcuEmulator->loadFirmware(firmwareFile)) {
            m_statusLabel->setText("Firmware Loaded");
            m_startBtn->setEnabled(true);
        } else {
            QMessageBox::warning(this, "Error", "Failed to load firmware");
        }
    }
}

void SmartMeterSimulator::startSimulation()
{
    if (m_firmwarePathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please load firmware first");
        return;
    }

    m_simulationRunning = true;
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_statusLabel->setText("Running");
    
    // Configure MCU emulator
    QString mcuFamily = m_mcuFamilyCombo->currentText();
    QString partNumber = m_mcuPartCombo->currentText();
    QString architecture = m_architectureCombo->currentText();
    
    m_mcuEmulator->configure(mcuFamily.toStdString(), 
                            partNumber.toStdString(), 
                            architecture.toStdString());
    
    // Configure metering engine
    bool isThreePhase = m_phaseConfigCombo->currentText() == "Three Phase";
    m_meteringEngine->configure(isThreePhase,
                               m_voltageAmplitude->value(),
                               m_currentAmplitude->value(),
                               m_frequency->value(),
                               m_powerFactor->value());
    
    // Start simulation
    m_core->startSimulation();
    m_systemLog->append("Simulation started");
}

void SmartMeterSimulator::stopSimulation()
{
    m_simulationRunning = false;
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_statusLabel->setText("Stopped");
    
    m_core->stopSimulation();
    m_systemLog->append("Simulation stopped");
}

void SmartMeterSimulator::resetSimulation()
{
    stopSimulation();
    m_mcuEmulator->reset();
    m_meteringEngine->reset();
    m_simulationProgress->setValue(0);
    m_systemLog->append("Simulation reset");
}

void SmartMeterSimulator::updateDisplay()
{
    if (!m_simulationRunning) return;

    // Update metering values
    auto measurements = m_meteringEngine->getMeasurements();
    
    if (m_meteringDisplay->item(0, 1)) {
        m_meteringDisplay->item(0, 1)->setText(QString::number(measurements.voltageRMS, 'f', 2) + " V");
    } else {
        m_meteringDisplay->setItem(0, 1, new QTableWidgetItem(QString::number(measurements.voltageRMS, 'f', 2) + " V"));
    }
    
    if (m_meteringDisplay->item(1, 1)) {
        m_meteringDisplay->item(1, 1)->setText(QString::number(measurements.currentRMS, 'f', 3) + " A");
    } else {
        m_meteringDisplay->setItem(1, 1, new QTableWidgetItem(QString::number(measurements.currentRMS, 'f', 3) + " A"));
    }
    
    if (m_meteringDisplay->item(2, 1)) {
        m_meteringDisplay->item(2, 1)->setText(QString::number(measurements.activePower, 'f', 2) + " W");
    } else {
        m_meteringDisplay->setItem(2, 1, new QTableWidgetItem(QString::number(measurements.activePower, 'f', 2) + " W"));
    }
    
    if (m_meteringDisplay->item(5, 1)) {
        m_meteringDisplay->item(5, 1)->setText(QString::number(measurements.powerFactor, 'f', 3));
    } else {
        m_meteringDisplay->setItem(5, 1, new QTableWidgetItem(QString::number(measurements.powerFactor, 'f', 3)));
    }
    
    if (m_meteringDisplay->item(6, 1)) {
        m_meteringDisplay->item(6, 1)->setText(QString::number(measurements.frequency, 'f', 2) + " Hz");
    } else {
        m_meteringDisplay->setItem(6, 1, new QTableWidgetItem(QString::number(measurements.frequency, 'f', 2) + " Hz"));
    }

    // Update waveforms
    updateWaveforms();
    
    // Update simulation progress
    static int progressCounter = 0;
    progressCounter = (progressCounter + 1) % 100;
    m_simulationProgress->setValue(progressCounter);
}

void SmartMeterSimulator::updateWaveforms()
{
    m_waveformScene->clear();
    
    const int width = 400;
    const int height = 150;
    const int samples = 100;
    
    QPen voltagePen(Qt::blue, 2);
    QPen currentPen(Qt::red, 2);
    
    // Generate voltage waveform
    for (int i = 0; i < samples - 1; i++) {
        double t1 = 2 * M_PI * i / samples;
        double t2 = 2 * M_PI * (i + 1) / samples;
        
        double v1 = m_voltageAmplitude->value() * sin(t1) * height / 500.0;
        double v2 = m_voltageAmplitude->value() * sin(t2) * height / 500.0;
        
        m_waveformScene->addLine(i * width / samples, height/2 - v1,
                                (i + 1) * width / samples, height/2 - v2, voltagePen);
        
        double c1 = m_currentAmplitude->value() * sin(t1 - acos(m_powerFactor->value())) * height / 20.0;
        double c2 = m_currentAmplitude->value() * sin(t2 - acos(m_powerFactor->value())) * height / 20.0;
        
        m_waveformScene->addLine(i * width / samples, height/2 - c1,
                                (i + 1) * width / samples, height/2 - c2, currentPen);
    }
    
    m_waveformView->fitInView(m_waveformScene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void SmartMeterSimulator::onProtocolCommand()
{
    QString command = m_protocolCommand->text();
    QString protocol = m_protocolCombo->currentText();
    
    if (command.isEmpty()) return;
    
    m_protocolLog->append(QString("[%1] TX: %2").arg(protocol, command));
    
    // Process command through protocol handler
    std::string response = m_protocolHandler->processCommand(protocol.toStdString(), command.toStdString());
    
    m_protocolResponse->setText(QString::fromStdString(response));
    m_protocolLog->append(QString("[%1] RX: %2").arg(protocol, QString::fromStdString(response)));
    
    m_protocolCommand->clear();
}

void SmartMeterSimulator::injectTamperEvent()
{
    QString tamperType = m_tamperTypeCombo->currentText();
    m_meteringEngine->injectTamperEvent(tamperType.toStdString());
    m_systemLog->append(QString("Tamper event injected: %1").arg(tamperType));
}

void SmartMeterSimulator::exportResults()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Results", "simulation_results.csv", "CSV Files (*.csv)");
    
    if (!fileName.isEmpty()) {
        std::ofstream file(fileName.toStdString());
        if (file.is_open()) {
            file << "Timestamp,Parameter,Value\n";
            // Export measurement data
            auto measurements = m_meteringEngine->getMeasurements();
            file << QDateTime::currentDateTime().toString().toStdString() << ",Voltage," << measurements.voltageRMS << "\n";
            file << QDateTime::currentDateTime().toString().toStdString() << ",Current," << measurements.currentRMS << "\n";
            file << QDateTime::currentDateTime().toString().toStdString() << ",Power," << measurements.activePower << "\n";
            file.close();
            m_systemLog->append(QString("Results exported to: %1").arg(fileName));
        }
    }
}

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    SmartMeterSimulator simulator;
    simulator.show();
    
    return app.exec();
}
