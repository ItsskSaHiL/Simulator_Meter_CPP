
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <vector>
#include <memory>

struct OscilloscopeTrace {
    std::vector<double> data;
    QString name;
    QColor color;
    double verticalScale;
    double verticalOffset;
    bool enabled;
};

struct MultimeterReading {
    QString parameter;
    double value;
    QString unit;
    QString range;
};

struct LogicAnalyzerChannel {
    std::vector<bool> data;
    QString name;
    bool enabled;
    QColor color;
};

class VirtualOscilloscope : public QWidget
{
    Q_OBJECT

public:
    VirtualOscilloscope(QWidget* parent = nullptr);
    
    void addTrace(const QString& name, const QColor& color);
    void updateTrace(const QString& name, const std::vector<double>& data);
    void setTimebase(double timePerDiv);
    void setVerticalScale(const QString& traceName, double voltsPerDiv);
    void setTriggerLevel(double level);

private slots:
    void onRunStopClicked();
    void onSingleShotClicked();
    void onClearClicked();
    void updateDisplay();

private:
    void setupUI();
    void setupControls();
    void drawWaveforms();
    void drawGrid();

    QGraphicsView* m_waveformView;
    QGraphicsScene* m_waveformScene;
    QTimer* m_updateTimer;
    
    std::map<QString, OscilloscopeTrace> m_traces;
    double m_timePerDiv;
    double m_triggerLevel;
    bool m_running;
    int m_sampleRate;
    
    // Controls
    QComboBox* m_timebaseCombo;
    QDoubleSpinBox* m_triggerSpin;
    QPushButton* m_runStopBtn;
    QPushButton* m_singleBtn;
    QPushButton* m_clearBtn;
};

class VirtualMultimeter : public QWidget
{
    Q_OBJECT

public:
    VirtualMultimeter(QWidget* parent = nullptr);
    
    void updateReading(const QString& parameter, double value, const QString& unit);
    void setMode(const QString& mode);
    void setRange(const QString& range);

private slots:
    void onModeChanged(const QString& mode);
    void onRangeChanged(const QString& range);
    void updateReadings();

private:
    void setupUI();
    void setupDisplay();

    QLabel* m_mainDisplay;
    QLabel* m_unitLabel;
    QLabel* m_modeLabel;
    QComboBox* m_modeCombo;
    QComboBox* m_rangeCombo;
    QTimer* m_updateTimer;
    
    std::map<QString, MultimeterReading> m_readings;
    QString m_currentMode;
    QString m_currentRange;
};

class VirtualLogicAnalyzer : public QWidget
{
    Q_OBJECT

public:
    VirtualLogicAnalyzer(QWidget* parent = nullptr);
    
    void addChannel(const QString& name, const QColor& color);
    void updateChannel(const QString& name, const std::vector<bool>& data);
    void setTimebase(double timePerDiv);
    void setTriggerChannel(const QString& channel);

private slots:
    void onRunStopClicked();
    void onClearClicked();
    void updateDisplay();

private:
    void setupUI();
    void drawChannels();
    void drawGrid();

    QGraphicsView* m_waveformView;
    QGraphicsScene* m_waveformScene;
    QTimer* m_updateTimer;
    
    std::map<QString, LogicAnalyzerChannel> m_channels;
    double m_timePerDiv;
    QString m_triggerChannel;
    bool m_running;
    
    // Controls
    QComboBox* m_timebaseCombo;
    QComboBox* m_triggerCombo;
    QPushButton* m_runStopBtn;
    QPushButton* m_clearBtn;
};

class MeasurementTools : public QTabWidget
{
    Q_OBJECT

public:
    MeasurementTools(QWidget* parent = nullptr);
    
    VirtualOscilloscope* getOscilloscope() { return m_oscilloscope; }
    VirtualMultimeter* getMultimeter() { return m_multimeter; }
    VirtualLogicAnalyzer* getLogicAnalyzer() { return m_logicAnalyzer; }

private:
    VirtualOscilloscope* m_oscilloscope;
    VirtualMultimeter* m_multimeter;
    VirtualLogicAnalyzer* m_logicAnalyzer;
};
