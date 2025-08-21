
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include "component_library.h"

class PropertyEditor : public QWidget
{
    Q_OBJECT

public:
    PropertyEditor(QWidget* parent = nullptr);
    
    void setComponent(ElectronicsComponent* component);
    void clearProperties();

signals:
    void propertyChanged(const QString& name, const QVariant& value);

private slots:
    void onDoubleValueChanged(double value);
    void onIntValueChanged(int value);
    void onStringValueChanged(const QString& value);
    void onBoolValueChanged(bool value);

private:
    void setupUI();
    void updateProperties();
    void addDoubleProperty(const QString& name, double value, double min = -1e6, double max = 1e6);
    void addIntProperty(const QString& name, int value, int min = -1000000, int max = 1000000);
    void addStringProperty(const QString& name, const QString& value, const QStringList& options = QStringList());
    void addBoolProperty(const QString& name, bool value);

    ElectronicsComponent* m_currentComponent;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QFormLayout* m_layout;
    
    std::map<QWidget*, QString> m_propertyMap;
};
