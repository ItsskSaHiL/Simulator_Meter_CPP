
#include "property_editor.h"

PropertyEditor::PropertyEditor(QWidget* parent)
    : QWidget(parent)
    , m_currentComponent(nullptr)
{
    setupUI();
}

void PropertyEditor::setupUI()
{
    setFixedWidth(250);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QLabel* titleLabel = new QLabel("Component Properties");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(titleLabel);
    
    m_scrollArea = new QScrollArea();
    m_contentWidget = new QWidget();
    m_layout = new QFormLayout(m_contentWidget);
    
    m_scrollArea->setWidget(m_contentWidget);
    m_scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(m_scrollArea);
}

void PropertyEditor::setComponent(ElectronicsComponent* component)
{
    m_currentComponent = component;
    updateProperties();
}

void PropertyEditor::clearProperties()
{
    // Clear existing widgets
    while (m_layout->count() > 0) {
        QLayoutItem* item = m_layout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_propertyMap.clear();
    m_currentComponent = nullptr;
}

void PropertyEditor::updateProperties()
{
    clearProperties();
    
    if (!m_currentComponent) {
        return;
    }
    
    // Component name and type
    addStringProperty("Name", m_currentComponent->getName());
    addStringProperty("Label", m_currentComponent->getLabel());
    
    const ComponentProperties& props = m_currentComponent->getProperties();
    
    // Add double properties
    for (const auto& pair : props.values) {
        addDoubleProperty(pair.first, pair.second);
    }
    
    // Add string properties
    for (const auto& pair : props.strings) {
        QStringList options;
        if (pair.first == "color") {
            options << "Red" << "Green" << "Blue" << "Yellow" << "White";
        } else if (pair.first == "type") {
            options << "Electrolytic" << "Ceramic" << "Tantalum" << "Film";
        } else if (pair.first == "waveform") {
            options << "Sine" << "Square" << "Triangle" << "Sawtooth";
        }
        addStringProperty(pair.first, pair.second, options);
    }
    
    // Add boolean properties
    for (const auto& pair : props.bools) {
        addBoolProperty(pair.first, pair.second);
    }
}

void PropertyEditor::addDoubleProperty(const QString& name, double value, double min, double max)
{
    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setRange(min, max);
    spinBox->setDecimals(6);
    spinBox->setValue(value);
    
    // Set appropriate step and suffix based on property name
    if (name.contains("resistance", Qt::CaseInsensitive)) {
        spinBox->setSuffix(" Ω");
        spinBox->setSingleStep(100);
    } else if (name.contains("capacitance", Qt::CaseInsensitive)) {
        spinBox->setSuffix(" F");
        spinBox->setSingleStep(1e-6);
    } else if (name.contains("voltage", Qt::CaseInsensitive)) {
        spinBox->setSuffix(" V");
        spinBox->setSingleStep(0.1);
    } else if (name.contains("current", Qt::CaseInsensitive)) {
        spinBox->setSuffix(" A");
        spinBox->setSingleStep(0.001);
    } else if (name.contains("frequency", Qt::CaseInsensitive)) {
        spinBox->setSuffix(" Hz");
        spinBox->setSingleStep(1);
    } else if (name.contains("power", Qt::CaseInsensitive)) {
        spinBox->setSuffix(" W");
        spinBox->setSingleStep(0.1);
    }
    
    m_propertyMap[spinBox] = name;
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyEditor::onDoubleValueChanged);
    
    m_layout->addRow(name + ":", spinBox);
}

void PropertyEditor::addIntProperty(const QString& name, int value, int min, int max)
{
    QSpinBox* spinBox = new QSpinBox();
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    
    m_propertyMap[spinBox] = name;
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PropertyEditor::onIntValueChanged);
    
    m_layout->addRow(name + ":", spinBox);
}

void PropertyEditor::addStringProperty(const QString& name, const QString& value, const QStringList& options)
{
    if (options.isEmpty()) {
        QLineEdit* lineEdit = new QLineEdit(value);
        m_propertyMap[lineEdit] = name;
        connect(lineEdit, &QLineEdit::textChanged,
                this, &PropertyEditor::onStringValueChanged);
        m_layout->addRow(name + ":", lineEdit);
    } else {
        QComboBox* comboBox = new QComboBox();
        comboBox->addItems(options);
        comboBox->setCurrentText(value);
        m_propertyMap[comboBox] = name;
        connect(comboBox, &QComboBox::currentTextChanged,
                this, &PropertyEditor::onStringValueChanged);
        m_layout->addRow(name + ":", comboBox);
    }
}

void PropertyEditor::addBoolProperty(const QString& name, bool value)
{
    QCheckBox* checkBox = new QCheckBox();
    checkBox->setChecked(value);
    
    m_propertyMap[checkBox] = name;
    connect(checkBox, &QCheckBox::toggled,
            this, &PropertyEditor::onBoolValueChanged);
    
    m_layout->addRow(name + ":", checkBox);
}

void PropertyEditor::onDoubleValueChanged(double value)
{
    QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox*>(sender());
    if (spinBox && m_propertyMap.contains(spinBox) && m_currentComponent) {
        QString propertyName = m_propertyMap[spinBox];
        m_currentComponent->setProperty(propertyName, value);
        emit propertyChanged(propertyName, value);
    }
}

void PropertyEditor::onIntValueChanged(int value)
{
    QSpinBox* spinBox = qobject_cast<QSpinBox*>(sender());
    if (spinBox && m_propertyMap.contains(spinBox) && m_currentComponent) {
        QString propertyName = m_propertyMap[spinBox];
        emit propertyChanged(propertyName, value);
    }
}

void PropertyEditor::onStringValueChanged(const QString& value)
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (widget && m_propertyMap.contains(widget) && m_currentComponent) {
        QString propertyName = m_propertyMap[widget];
        m_currentComponent->setProperty(propertyName, value);
        emit propertyChanged(propertyName, value);
    }
}

void PropertyEditor::onBoolValueChanged(bool value)
{
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
    if (checkBox && m_propertyMap.contains(checkBox) && m_currentComponent) {
        QString propertyName = m_propertyMap[checkBox];
        m_currentComponent->setProperty(propertyName, value);
        emit propertyChanged(propertyName, value);
    }
}

#include "property_editor.moc"
