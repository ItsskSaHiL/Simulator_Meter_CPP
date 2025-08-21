
#include "component_library.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

ElectronicsComponent::ElectronicsComponent(ComponentType type, const QString& name, QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
    , m_type(type)
    , m_name(name)
    , m_label(name)
    , m_dragging(false)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    initializeComponent();
    createPins();
    setPixmap(generatePixmap());
}

void ElectronicsComponent::initializeComponent()
{
    switch (m_type) {
        case ComponentType::RESISTOR:
            m_properties.values["resistance"] = 1000.0; // 1k ohm
            m_properties.values["tolerance"] = 5.0; // 5%
            m_properties.values["power"] = 0.25; // 0.25W
            break;
            
        case ComponentType::CAPACITOR:
            m_properties.values["capacitance"] = 100e-6; // 100uF
            m_properties.values["voltage"] = 25.0; // 25V
            m_properties.strings["type"] = "Electrolytic";
            break;
            
        case ComponentType::LED:
            m_properties.values["forward_voltage"] = 2.1; // 2.1V
            m_properties.values["forward_current"] = 0.02; // 20mA
            m_properties.strings["color"] = "Red";
            break;
            
        case ComponentType::POWER_DC:
            m_properties.values["voltage"] = 5.0; // 5V
            m_properties.values["current_limit"] = 1.0; // 1A
            break;
            
        case ComponentType::POWER_AC:
            m_properties.values["voltage_rms"] = 230.0; // 230V RMS
            m_properties.values["frequency"] = 50.0; // 50Hz
            break;
            
        case ComponentType::SIGNAL_GEN:
            m_properties.values["amplitude"] = 5.0; // 5V
            m_properties.values["frequency"] = 1000.0; // 1kHz
            m_properties.strings["waveform"] = "Sine";
            break;
            
        default:
            break;
    }
}

void ElectronicsComponent::createPins()
{
    m_pins.clear();
    
    switch (m_type) {
        case ComponentType::RESISTOR:
        case ComponentType::CAPACITOR:
        case ComponentType::INDUCTOR:
            m_pins.push_back({0, "Pin1", QPointF(-20, 0), false, ""});
            m_pins.push_back({1, "Pin2", QPointF(20, 0), false, ""});
            break;
            
        case ComponentType::LED:
        case ComponentType::DIODE:
            m_pins.push_back({0, "Anode", QPointF(-20, 0), false, ""});
            m_pins.push_back({1, "Cathode", QPointF(20, 0), false, ""});
            break;
            
        case ComponentType::TRANSISTOR_NPN:
        case ComponentType::TRANSISTOR_PNP:
            m_pins.push_back({0, "Collector", QPointF(0, -20), false, ""});
            m_pins.push_back({1, "Base", QPointF(-20, 0), false, ""});
            m_pins.push_back({2, "Emitter", QPointF(0, 20), false, ""});
            break;
            
        case ComponentType::OPAMP:
            m_pins.push_back({0, "V+", QPointF(-25, -10), false, ""});
            m_pins.push_back({1, "V-", QPointF(-25, 10), false, ""});
            m_pins.push_back({2, "Vout", QPointF(25, 0), false, ""});
            m_pins.push_back({3, "VCC", QPointF(0, -20), false, ""});
            m_pins.push_back({4, "VEE", QPointF(0, 20), false, ""});
            break;
            
        case ComponentType::POWER_DC:
        case ComponentType::POWER_AC:
            m_pins.push_back({0, "Positive", QPointF(0, -15), false, ""});
            m_pins.push_back({1, "Negative", QPointF(0, 15), false, ""});
            break;
            
        default:
            // Default 2-pin component
            m_pins.push_back({0, "Pin1", QPointF(-15, 0), false, ""});
            m_pins.push_back({1, "Pin2", QPointF(15, 0), false, ""});
            break;
    }
}

QPixmap ElectronicsComponent::generatePixmap()
{
    QPixmap pixmap(60, 40);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect rect = pixmap.rect().adjusted(10, 10, -10, -10);
    
    switch (m_type) {
        case ComponentType::RESISTOR:
            painter.setPen(QPen(Qt::black, 2));
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignCenter, "R");
            break;
            
        case ComponentType::CAPACITOR:
            painter.setPen(QPen(Qt::black, 2));
            painter.drawLine(rect.left() + 5, rect.top(), rect.left() + 5, rect.bottom());
            painter.drawLine(rect.right() - 5, rect.top(), rect.right() - 5, rect.bottom());
            painter.drawText(rect, Qt::AlignCenter, "C");
            break;
            
        case ComponentType::LED:
            painter.setPen(QPen(Qt::red, 2));
            painter.setBrush(Qt::red);
            painter.drawEllipse(rect);
            painter.setPen(Qt::white);
            painter.drawText(rect, Qt::AlignCenter, "LED");
            break;
            
        case ComponentType::POWER_DC:
            painter.setPen(QPen(Qt::blue, 2));
            painter.setBrush(Qt::lightGray);
            painter.drawEllipse(rect);
            painter.setPen(Qt::black);
            painter.drawText(rect, Qt::AlignCenter, "DC");
            break;
            
        case ComponentType::POWER_AC:
            painter.setPen(QPen(Qt::red, 2));
            painter.setBrush(Qt::lightGray);
            painter.drawEllipse(rect);
            painter.setPen(Qt::black);
            painter.drawText(rect, Qt::AlignCenter, "AC");
            break;
            
        default:
            painter.setPen(QPen(Qt::black, 2));
            painter.setBrush(Qt::lightGray);
            painter.drawRect(rect);
            painter.drawText(rect, Qt::AlignCenter, m_name.left(3));
            break;
    }
    
    return pixmap;
}

QRectF ElectronicsComponent::boundingRect() const
{
    return QRectF(-30, -20, 60, 40);
}

void ElectronicsComponent::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QGraphicsPixmapItem::paint(painter, option, widget);
    
    // Draw pins
    painter->setPen(QPen(Qt::black, 1));
    for (const auto& pin : m_pins) {
        painter->drawEllipse(pin.position, 2, 2);
    }
    
    // Draw selection highlight
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter->drawRect(boundingRect());
    }
}

ComponentPin* ElectronicsComponent::getPin(int id)
{
    for (auto& pin : m_pins) {
        if (pin.id == id) {
            return &pin;
        }
    }
    return nullptr;
}

void ElectronicsComponent::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStart = event->pos();
    }
    QGraphicsPixmapItem::mousePressEvent(event);
}

void ElectronicsComponent::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_dragging) {
        QPointF delta = event->pos() - m_dragStart;
        setPos(pos() + delta);
    }
    QGraphicsPixmapItem::mouseMoveEvent(event);
}

void ElectronicsComponent::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    m_dragging = false;
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

ComponentLibrary::ComponentLibrary(QWidget* parent)
    : QWidget(parent)
{
    setupLibrary();
}

void ComponentLibrary::setupLibrary()
{
    setFixedWidth(200);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QScrollArea* scrollArea = new QScrollArea();
    QWidget* scrollWidget = new QWidget();
    m_layout = new QGridLayout(scrollWidget);
    
    // Passive Components
    QGroupBox* passiveGroup = new QGroupBox("Passive Components");
    QGridLayout* passiveLayout = new QGridLayout(passiveGroup);
    
    addComponentButton(ComponentType::RESISTOR, "Resistor", "R");
    addComponentButton(ComponentType::CAPACITOR, "Capacitor", "C");
    addComponentButton(ComponentType::INDUCTOR, "Inductor", "L");
    
    // Active Components
    QGroupBox* activeGroup = new QGroupBox("Active Components");
    QGridLayout* activeLayout = new QGridLayout(activeGroup);
    
    addComponentButton(ComponentType::DIODE, "Diode", "D");
    addComponentButton(ComponentType::LED, "LED", "LED");
    addComponentButton(ComponentType::TRANSISTOR_NPN, "NPN", "Q");
    addComponentButton(ComponentType::OPAMP, "OpAmp", "U");
    
    // Power Sources
    QGroupBox* powerGroup = new QGroupBox("Power Sources");
    QGridLayout* powerLayout = new QGridLayout(powerGroup);
    
    addComponentButton(ComponentType::POWER_DC, "DC Source", "V");
    addComponentButton(ComponentType::POWER_AC, "AC Source", "~");
    addComponentButton(ComponentType::SIGNAL_GEN, "Signal Gen", "SG");
    
    // Communication Modules
    QGroupBox* commGroup = new QGroupBox("Communication");
    QGridLayout* commLayout = new QGridLayout(commGroup);
    
    addComponentButton(ComponentType::GSM_MODULE, "GSM", "GSM");
    addComponentButton(ComponentType::BLE_MODULE, "BLE", "BLE");
    addComponentButton(ComponentType::WIFI_MODULE, "WiFi", "WiFi");
    
    m_layout->addWidget(passiveGroup, 0, 0);
    m_layout->addWidget(activeGroup, 1, 0);
    m_layout->addWidget(powerGroup, 2, 0);
    m_layout->addWidget(commGroup, 3, 0);
    
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);
}

void ComponentLibrary::addComponentButton(ComponentType type, const QString& name, const QString& icon)
{
    QPushButton* button = new QPushButton(name);
    button->setFixedSize(80, 30);
    button->setToolTip(name);
    
    m_buttonMap[button] = type;
    connect(button, &QPushButton::clicked, this, &ComponentLibrary::onComponentClicked);
    
    int row = m_layout->rowCount();
    m_layout->addWidget(button, row, 0);
}

void ComponentLibrary::onComponentClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button && m_buttonMap.contains(button)) {
        ComponentType type = m_buttonMap[button];
        emit componentSelected(type, button->text());
    }
}

CircuitCanvas::CircuitCanvas(QWidget* parent)
    : QGraphicsView(parent)
    , m_selectedComponent(nullptr)
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-1000, -1000, 2000, 2000);
    setScene(m_scene);
    
    setAcceptDrops(true);
    setDragMode(QGraphicsView::RubberBandDrag);
    
    // Grid background
    setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    
    connect(m_scene, &QGraphicsScene::selectionChanged, 
            this, &CircuitCanvas::onSceneSelectionChanged);
}

void CircuitCanvas::addComponent(ComponentType type, const QString& name, const QPointF& position)
{
    ElectronicsComponent* component = new ElectronicsComponent(type, name);
    component->setPos(position);
    
    m_scene->addItem(component);
    m_components.push_back(component);
    
    emit componentAdded(component);
}

void CircuitCanvas::removeComponent(ElectronicsComponent* component)
{
    auto it = std::find(m_components.begin(), m_components.end(), component);
    if (it != m_components.end()) {
        m_components.erase(it);
        m_scene->removeItem(component);
        emit componentRemoved(component);
        delete component;
    }
}

void CircuitCanvas::clearCanvas()
{
    for (auto* component : m_components) {
        m_scene->removeItem(component);
        delete component;
    }
    m_components.clear();
}

void CircuitCanvas::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}

void CircuitCanvas::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void CircuitCanvas::dropEvent(QDropEvent* event)
{
    QString componentData = event->mimeData()->text();
    QStringList parts = componentData.split("|");
    
    if (parts.size() >= 2) {
        ComponentType type = static_cast<ComponentType>(parts[0].toInt());
        QString name = parts[1];
        QPointF position = mapToScene(event->pos());
        
        addComponent(type, name, position);
        event->acceptProposedAction();
    }
}

void CircuitCanvas::mousePressEvent(QMouseEvent* event)
{
    QGraphicsView::mousePressEvent(event);
}

void CircuitCanvas::onSceneSelectionChanged()
{
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    
    if (!selected.isEmpty()) {
        ElectronicsComponent* component = qgraphicsitem_cast<ElectronicsComponent*>(selected.first());
        if (component && component != m_selectedComponent) {
            m_selectedComponent = component;
            emit componentSelected(component);
        }
    } else {
        m_selectedComponent = nullptr;
        emit componentSelected(nullptr);
    }
}

#include "component_library.moc"
