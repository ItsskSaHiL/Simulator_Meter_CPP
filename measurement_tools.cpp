
#include "measurement_tools.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <cmath>

VirtualOscilloscope::VirtualOscilloscope(QWidget* parent)
    : QWidget(parent)
    , m_timePerDiv(1e-3)
    , m_triggerLevel(0.0)
    , m_running(false)
    , m_sampleRate(1000000)
{
    setupUI();
    setupControls();
    
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &VirtualOscilloscope::updateDisplay);
    m_updateTimer->start(100); // 10 FPS
}

void VirtualOscilloscope::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    m_waveformView = new QGraphicsView();
    m_waveformScene = new QGraphicsScene();
    m_waveformView->setScene(m_waveformScene);
    m_waveformView->setMinimumSize(600, 400);
    
    layout->addWidget(m_waveformView);
}

void VirtualOscilloscope::setupControls()
{
    QHBoxLayout* controlLayout = new QHBoxLayout();
    
    controlLayout->addWidget(new QLabel("Timebase:"));
    m_timebaseCombo = new QComboBox();
    m_timebaseCombo->addItems({"1µs/div", "10µs/div", "100µs/div", "1ms/div", "10ms/div", "100ms/div"});
    m_timebaseCombo->setCurrentText("1ms/div");
    controlLayout->addWidget(m_timebaseCombo);
    
    controlLayout->addWidget(new QLabel("Trigger:"));
    m_triggerSpin = new QDoubleSpinBox();
    m_triggerSpin->setRange(-10, 10);
    m_triggerSpin->setSingleStep(0.1);
    m_triggerSpin->setSuffix(" V");
    controlLayout->addWidget(m_triggerSpin);
    
    m_runStopBtn = new QPushButton("Run");
    connect(m_runStopBtn, &QPushButton::clicked, this, &VirtualOscilloscope::onRunStopClicked);
    controlLayout->addWidget(m_runStopBtn);
    
    m_singleBtn = new QPushButton("Single");
    connect(m_singleBtn, &QPushButton::clicked, this, &VirtualOscilloscope::onSingleShotClicked);
    controlLayout->addWidget(m_singleBtn);
    
    m_clearBtn = new QPushButton("Clear");
    connect(m_clearBtn, &QPushButton::clicked, this, &VirtualOscilloscope::onClearClicked);
    controlLayout->addWidget(m_clearBtn);
    
    controlLayout->addStretch();
    
    layout()->addItem(controlLayout);
}

void VirtualOscilloscope::addTrace(const QString& name, const QColor& color)
{
    OscilloscopeTrace trace;
    trace.name = name;
    trace.color = color;
    trace.verticalScale = 1.0;
    trace.verticalOffset = 0.0;
    trace.enabled = true;
    
    m_traces[name] = trace;
}

void VirtualOscilloscope::updateTrace(const QString& name, const std::vector<double>& data)
{
    if (m_traces.contains(name)) {
        m_traces[name].data = data;
    }
}

void VirtualOscilloscope::onRunStopClicked()
{
    m_running = !m_running;
    m_runStopBtn->setText(m_running ? "Stop" : "Run");
}

void VirtualOscilloscope::onSingleShotClicked()
{
    // Trigger single acquisition
    updateDisplay();
}

void VirtualOscilloscope::onClearClicked()
{
    for (auto& trace : m_traces) {
        trace.data.clear();
    }
    m_waveformScene->clear();
    drawGrid();
}

void VirtualOscilloscope::updateDisplay()
{
    if (!m_running) return;
    
    m_waveformScene->clear();
    drawGrid();
    drawWaveforms();
}

void VirtualOscilloscope::drawGrid()
{
    QRectF sceneRect = m_waveformScene->sceneRect();
    if (sceneRect.isEmpty()) {
        sceneRect = QRectF(-300, -200, 600, 400);
        m_waveformScene->setSceneRect(sceneRect);
    }
    
    QPen gridPen(Qt::gray, 1, Qt::DotLine);
    
    // Vertical grid lines
    for (int i = -5; i <= 5; i++) {
        double x = i * sceneRect.width() / 10;
        m_waveformScene->addLine(x, sceneRect.top(), x, sceneRect.bottom(), gridPen);
    }
    
    // Horizontal grid lines
    for (int i = -4; i <= 4; i++) {
        double y = i * sceneRect.height() / 8;
        m_waveformScene->addLine(sceneRect.left(), y, sceneRect.right(), y, gridPen);
    }
    
    // Center lines
    QPen centerPen(Qt::black, 2);
    m_waveformScene->addLine(0, sceneRect.top(), 0, sceneRect.bottom(), centerPen);
    m_waveformScene->addLine(sceneRect.left(), 0, sceneRect.right(), 0, centerPen);
}

void VirtualOscilloscope::drawWaveforms()
{
    QRectF sceneRect = m_waveformScene->sceneRect();
    
    for (const auto& trace : m_traces) {
        if (!trace.enabled || trace.data.empty()) continue;
        
        QPen tracePen(trace.color, 2);
        
        for (size_t i = 0; i < trace.data.size() - 1; i++) {
            double x1 = sceneRect.left() + (double(i) / trace.data.size()) * sceneRect.width();
            double x2 = sceneRect.left() + (double(i + 1) / trace.data.size()) * sceneRect.width();
            double y1 = -trace.data[i] * 50 + trace.verticalOffset;
            double y2 = -trace.data[i + 1] * 50 + trace.verticalOffset;
            
            m_waveformScene->addLine(x1, y1, x2, y2, tracePen);
        }
    }
}

VirtualMultimeter::VirtualMultimeter(QWidget* parent)
    : QWidget(parent)
    , m_currentMode("DCV")
    , m_currentRange("Auto")
{
    setupUI();
    setupDisplay();
    
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &VirtualMultimeter::updateReadings);
    m_updateTimer->start(500); // 2 Hz update rate
}

void VirtualMultimeter::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Main display
    m_mainDisplay = new QLabel("0.000");
    m_mainDisplay->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; background: black; color: green; padding: 10px; }");
    m_mainDisplay->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_mainDisplay);
    
    // Unit and mode labels
    QHBoxLayout* infoLayout = new QHBoxLayout();
    m_unitLabel = new QLabel("V");
    m_unitLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_modeLabel = new QLabel("DC");
    m_modeLabel->setStyleSheet("font-size: 16px;");
    
    infoLayout->addWidget(m_unitLabel);
    infoLayout->addWidget(m_modeLabel);
    infoLayout->addStretch();
    layout->addLayout(infoLayout);
    
    // Controls
    QGridLayout* controlLayout = new QGridLayout();
    
    controlLayout->addWidget(new QLabel("Mode:"), 0, 0);
    m_modeCombo = new QComboBox();
    m_modeCombo->addItems({"DCV", "ACV", "DCA", "ACA", "Resistance", "Frequency", "Capacitance"});
    connect(m_modeCombo, &QComboBox::currentTextChanged, this, &VirtualMultimeter::onModeChanged);
    controlLayout->addWidget(m_modeCombo, 0, 1);
    
    controlLayout->addWidget(new QLabel("Range:"), 1, 0);
    m_rangeCombo = new QComboBox();
    m_rangeCombo->addItems({"Auto", "200mV", "2V", "20V", "200V", "1000V"});
    connect(m_rangeCombo, &QComboBox::currentTextChanged, this, &VirtualMultimeter::onRangeChanged);
    controlLayout->addWidget(m_rangeCombo, 1, 1);
    
    layout->addLayout(controlLayout);
    layout->addStretch();
}

void VirtualMultimeter::setupDisplay()
{
    m_readings["Voltage"] = {"Voltage", 0.0, "V", "Auto"};
    m_readings["Current"] = {"Current", 0.0, "A", "Auto"};
    m_readings["Resistance"] = {"Resistance", 0.0, "Ω", "Auto"};
    m_readings["Frequency"] = {"Frequency", 0.0, "Hz", "Auto"};
    m_readings["Power"] = {"Power", 0.0, "W", "Auto"};
}

void VirtualMultimeter::updateReading(const QString& parameter, double value, const QString& unit)
{
    if (m_readings.contains(parameter)) {
        m_readings[parameter].value = value;
        m_readings[parameter].unit = unit;
    }
}

void VirtualMultimeter::onModeChanged(const QString& mode)
{
    m_currentMode = mode;
}

void VirtualMultimeter::onRangeChanged(const QString& range)
{
    m_currentRange = range;
}

void VirtualMultimeter::updateReadings()
{
    QString parameter;
    if (m_currentMode.contains("V")) parameter = "Voltage";
    else if (m_currentMode.contains("A")) parameter = "Current";
    else if (m_currentMode == "Resistance") parameter = "Resistance";
    else if (m_currentMode == "Frequency") parameter = "Frequency";
    
    if (m_readings.contains(parameter)) {
        double value = m_readings[parameter].value;
        QString unit = m_readings[parameter].unit;
        
        m_mainDisplay->setText(QString::number(value, 'f', 3));
        m_unitLabel->setText(unit);
        m_modeLabel->setText(m_currentMode);
    }
}

MeasurementTools::MeasurementTools(QWidget* parent)
    : QTabWidget(parent)
{
    m_oscilloscope = new VirtualOscilloscope();
    m_multimeter = new VirtualMultimeter();
    m_logicAnalyzer = new VirtualLogicAnalyzer();
    
    addTab(m_oscilloscope, "Oscilloscope");
    addTab(m_multimeter, "Multimeter");
    addTab(m_logicAnalyzer, "Logic Analyzer");
    
    // Initialize oscilloscope traces
    m_oscilloscope->addTrace("Voltage", Qt::blue);
    m_oscilloscope->addTrace("Current", Qt::red);
}

// Simplified VirtualLogicAnalyzer implementation
VirtualLogicAnalyzer::VirtualLogicAnalyzer(QWidget* parent)
    : QWidget(parent)
    , m_timePerDiv(1e-3)
    , m_running(false)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    m_waveformView = new QGraphicsView();
    m_waveformScene = new QGraphicsScene();
    m_waveformView->setScene(m_waveformScene);
    m_waveformView->setMinimumSize(600, 300);
    
    layout->addWidget(m_waveformView);
    
    // Simple controls
    QHBoxLayout* controlLayout = new QHBoxLayout();
    m_runStopBtn = new QPushButton("Run");
    connect(m_runStopBtn, &QPushButton::clicked, this, &VirtualLogicAnalyzer::onRunStopClicked);
    controlLayout->addWidget(m_runStopBtn);
    
    layout->addLayout(controlLayout);
}

void VirtualLogicAnalyzer::addChannel(const QString& name, const QColor& color)
{
    LogicAnalyzerChannel channel;
    channel.name = name;
    channel.color = color;
    channel.enabled = true;
    m_channels[name] = channel;
}

void VirtualLogicAnalyzer::updateChannel(const QString& name, const std::vector<bool>& data)
{
    if (m_channels.contains(name)) {
        m_channels[name].data = data;
    }
}

void VirtualLogicAnalyzer::onRunStopClicked()
{
    m_running = !m_running;
    m_runStopBtn->setText(m_running ? "Stop" : "Run");
}

void VirtualLogicAnalyzer::onClearClicked()
{
    for (auto& channel : m_channels) {
        channel.data.clear();
    }
    m_waveformScene->clear();
}

void VirtualLogicAnalyzer::updateDisplay()
{
    // Simplified display update
}

void VirtualLogicAnalyzer::drawChannels()
{
    // Simplified channel drawing
}

void VirtualLogicAnalyzer::drawGrid()
{
    // Simplified grid drawing
}

#include "measurement_tools.moc"
