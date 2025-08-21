
#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QMouseEvent>
#include <vector>
#include <map>
#include <string>

enum class ComponentType {
    RESISTOR,
    CAPACITOR,
    INDUCTOR,
    DIODE,
    LED,
    TRANSISTOR_NPN,
    TRANSISTOR_PNP,
    OPAMP,
    SWITCH,
    RELAY,
    POWER_DC,
    POWER_AC,
    SIGNAL_GEN,
    GSM_MODULE,
    BLE_MODULE,
    WIFI_MODULE,
    VOLTAGE_SOURCE,
    CURRENT_SOURCE
};

struct ComponentPin {
    int id;
    QString name;
    QPointF position;
    bool isConnected;
    QString connectedNet;
};

struct ComponentProperties {
    std::map<QString, double> values;
    std::map<QString, QString> strings;
    std::map<QString, bool> bools;
};

class ElectronicsComponent : public QGraphicsPixmapItem
{
public:
    ElectronicsComponent(ComponentType type, const QString& name, QGraphicsItem* parent = nullptr);
    
    ComponentType getType() const { return m_type; }
    QString getName() const { return m_name; }
    QString getLabel() const { return m_label; }
    void setLabel(const QString& label) { m_label = label; }
    
    const std::vector<ComponentPin>& getPins() const { return m_pins; }
    ComponentPin* getPin(int id);
    
    ComponentProperties& getProperties() { return m_properties; }
    const ComponentProperties& getProperties() const { return m_properties; }
    
    void setProperty(const QString& key, double value) { m_properties.values[key] = value; }
    void setProperty(const QString& key, const QString& value) { m_properties.strings[key] = value; }
    void setProperty(const QString& key, bool value) { m_properties.bools[key] = value; }
    
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void initializeComponent();
    void createPins();
    QPixmap generatePixmap();

    ComponentType m_type;
    QString m_name;
    QString m_label;
    std::vector<ComponentPin> m_pins;
    ComponentProperties m_properties;
    bool m_dragging;
    QPointF m_dragStart;
};

class ComponentLibrary : public QWidget
{
    Q_OBJECT

public:
    ComponentLibrary(QWidget* parent = nullptr);

signals:
    void componentSelected(ComponentType type, const QString& name);

private slots:
    void onComponentClicked();

private:
    void setupLibrary();
    void addComponentButton(ComponentType type, const QString& name, const QString& icon);

    QGridLayout* m_layout;
    std::map<QPushButton*, ComponentType> m_buttonMap;
};

class CircuitCanvas : public QGraphicsView
{
    Q_OBJECT

public:
    CircuitCanvas(QWidget* parent = nullptr);
    
    void addComponent(ComponentType type, const QString& name, const QPointF& position);
    void removeComponent(ElectronicsComponent* component);
    void clearCanvas();
    
    std::vector<ElectronicsComponent*> getComponents() const { return m_components; }

signals:
    void componentAdded(ElectronicsComponent* component);
    void componentRemoved(ElectronicsComponent* component);
    void componentSelected(ElectronicsComponent* component);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onSceneSelectionChanged();

private:
    QGraphicsScene* m_scene;
    std::vector<ElectronicsComponent*> m_components;
    ElectronicsComponent* m_selectedComponent;
};
