
// Circuit Design Canvas
class CircuitDesigner {
    constructor(canvasId) {
        this.canvas = new fabric.Canvas(canvasId);
        this.canvas.setBackgroundColor('#f8f9fa', this.canvas.renderAll.bind(this.canvas));
        this.components = [];
        this.connections = [];
        this.selectedComponent = null;
        
        this.setupCanvas();
        this.addGrid();
    }

    setupCanvas() {
        this.canvas.on('object:selected', (e) => {
            this.selectedComponent = e.target;
            this.showPropertyEditor(e.target);
        });

        this.canvas.on('selection:cleared', () => {
            this.selectedComponent = null;
            this.hidePropertyEditor();
        });
    }

    addGrid() {
        const gridSize = 20;
        const width = this.canvas.getWidth();
        const height = this.canvas.getHeight();

        // Add vertical lines
        for (let i = 0; i <= width; i += gridSize) {
            const line = new fabric.Line([i, 0, i, height], {
                stroke: '#e0e0e0',
                strokeWidth: 1,
                selectable: false,
                evented: false,
                excludeFromExport: true
            });
            this.canvas.add(line);
        }

        // Add horizontal lines
        for (let i = 0; i <= height; i += gridSize) {
            const line = new fabric.Line([0, i, width, i], {
                stroke: '#e0e0e0',
                strokeWidth: 1,
                selectable: false,
                evented: false,
                excludeFromExport: true
            });
            this.canvas.add(line);
        }
    }

    addComponent(type, x = 100, y = 100) {
        let component;
        
        switch (type) {
            case 'resistor':
                component = this.createResistor(x, y);
                break;
            case 'capacitor':
                component = this.createCapacitor(x, y);
                break;
            case 'inductor':
                component = this.createInductor(x, y);
                break;
            case 'diode':
                component = this.createDiode(x, y);
                break;
            case 'led':
                component = this.createLED(x, y);
                break;
            case 'transistor':
                component = this.createTransistor(x, y);
                break;
            case 'dc-source':
                component = this.createDCSource(x, y);
                break;
            case 'ac-source':
                component = this.createACSource(x, y);
                break;
            default:
                component = this.createGenericComponent(type, x, y);
        }

        if (component) {
            component.componentType = type;
            component.componentId = Date.now();
            this.canvas.add(component);
            this.components.push(component);
            return component;
        }
    }

    createResistor(x, y) {
        const rect = new fabric.Rect({
            left: x,
            top: y,
            width: 60,
            height: 20,
            fill: 'white',
            stroke: 'black',
            strokeWidth: 2
        });

        const text = new fabric.Text('R', {
            left: x + 25,
            top: y + 5,
            fontSize: 12,
            fill: 'black'
        });

        return new fabric.Group([rect, text], {
            left: x,
            top: y
        });
    }

    createCapacitor(x, y) {
        const line1 = new fabric.Line([0, 0, 0, 40], {
            stroke: 'black',
            strokeWidth: 3
        });

        const line2 = new fabric.Line([10, 0, 10, 40], {
            stroke: 'black',
            strokeWidth: 3
        });

        const text = new fabric.Text('C', {
            left: 15,
            top: 15,
            fontSize: 12,
            fill: 'black'
        });

        return new fabric.Group([line1, line2, text], {
            left: x,
            top: y
        });
    }

    createInductor(x, y) {
        const coil = new fabric.Path('M 0 20 Q 10 0 20 20 Q 30 40 40 20 Q 50 0 60 20', {
            stroke: 'black',
            strokeWidth: 2,
            fill: 'transparent'
        });

        const text = new fabric.Text('L', {
            left: 25,
            top: 25,
            fontSize: 12,
            fill: 'black'
        });

        return new fabric.Group([coil, text], {
            left: x,
            top: y
        });
    }

    createDiode(x, y) {
        const triangle = new fabric.Triangle({
            width: 20,
            height: 20,
            fill: 'black',
            left: 0,
            top: 10
        });

        const line = new fabric.Line([20, 0, 20, 30], {
            stroke: 'black',
            strokeWidth: 3
        });

        return new fabric.Group([triangle, line], {
            left: x,
            top: y
        });
    }

    createLED(x, y) {
        const circle = new fabric.Circle({
            radius: 15,
            fill: 'red',
            stroke: 'darkred',
            strokeWidth: 2
        });

        const text = new fabric.Text('LED', {
            left: -10,
            top: -5,
            fontSize: 10,
            fill: 'white'
        });

        return new fabric.Group([circle, text], {
            left: x,
            top: y
        });
    }

    createTransistor(x, y) {
        const circle = new fabric.Circle({
            radius: 20,
            fill: 'lightgray',
            stroke: 'black',
            strokeWidth: 2
        });

        const text = new fabric.Text('Q', {
            left: -5,
            top: -7,
            fontSize: 12,
            fill: 'black'
        });

        return new fabric.Group([circle, text], {
            left: x,
            top: y
        });
    }

    createDCSource(x, y) {
        const circle = new fabric.Circle({
            radius: 20,
            fill: 'lightblue',
            stroke: 'blue',
            strokeWidth: 2
        });

        const text = new fabric.Text('DC', {
            left: -8,
            top: -7,
            fontSize: 10,
            fill: 'black'
        });

        return new fabric.Group([circle, text], {
            left: x,
            top: y
        });
    }

    createACSource(x, y) {
        const circle = new fabric.Circle({
            radius: 20,
            fill: 'lightcoral',
            stroke: 'red',
            strokeWidth: 2
        });

        const text = new fabric.Text('AC', {
            left: -8,
            top: -7,
            fontSize: 10,
            fill: 'black'
        });

        return new fabric.Group([circle, text], {
            left: x,
            top: y
        });
    }

    createGenericComponent(type, x, y) {
        const rect = new fabric.Rect({
            left: x,
            top: y,
            width: 40,
            height: 40,
            fill: 'lightgray',
            stroke: 'black',
            strokeWidth: 1
        });

        const text = new fabric.Text(type.substring(0, 3).toUpperCase(), {
            left: x + 10,
            top: y + 15,
            fontSize: 10,
            fill: 'black'
        });

        return new fabric.Group([rect, text], {
            left: x,
            top: y
        });
    }

    showPropertyEditor(component) {
        const panel = document.getElementById('propertyPanel');
        if (!panel) return;

        const type = component.componentType || 'unknown';
        
        panel.innerHTML = `
            <h4>Component Properties</h4>
            <div class="property-group">
                <label>Type:</label>
                <span>${type}</span>
            </div>
            <div class="property-group">
                <label>ID:</label>
                <span>${component.componentId || 'N/A'}</span>
            </div>
            <div class="property-group">
                <label>Position:</label>
                <span>X: ${Math.round(component.left)}, Y: ${Math.round(component.top)}</span>
            </div>
            ${this.getTypeSpecificProperties(type)}
            <button class="btn btn-danger btn-sm" onclick="circuitDesigner.deleteComponent()">Delete</button>
        `;
    }

    getTypeSpecificProperties(type) {
        switch (type) {
            case 'resistor':
                return `
                    <div class="property-group">
                        <label>Resistance (Ω):</label>
                        <input type="number" value="1000" class="property-input">
                    </div>
                    <div class="property-group">
                        <label>Tolerance (%):</label>
                        <input type="number" value="5" class="property-input">
                    </div>
                `;
            case 'capacitor':
                return `
                    <div class="property-group">
                        <label>Capacitance (μF):</label>
                        <input type="number" value="100" class="property-input">
                    </div>
                    <div class="property-group">
                        <label>Voltage (V):</label>
                        <input type="number" value="25" class="property-input">
                    </div>
                `;
            case 'dc-source':
                return `
                    <div class="property-group">
                        <label>Voltage (V):</label>
                        <input type="number" value="5" class="property-input">
                    </div>
                `;
            case 'ac-source':
                return `
                    <div class="property-group">
                        <label>Voltage RMS (V):</label>
                        <input type="number" value="230" class="property-input">
                    </div>
                    <div class="property-group">
                        <label>Frequency (Hz):</label>
                        <input type="number" value="50" class="property-input">
                    </div>
                `;
            default:
                return '';
        }
    }

    hidePropertyEditor() {
        const panel = document.getElementById('propertyPanel');
        if (panel) {
            panel.innerHTML = '<p>Select a component to edit properties</p>';
        }
    }

    deleteComponent() {
        if (this.selectedComponent) {
            this.canvas.remove(this.selectedComponent);
            this.components = this.components.filter(c => c !== this.selectedComponent);
            this.selectedComponent = null;
            this.hidePropertyEditor();
        }
    }

    clearCanvas() {
        this.canvas.clear();
        this.components = [];
        this.connections = [];
        this.addGrid();
    }
}

// Initialize circuit designer when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    if (document.getElementById('circuitCanvas')) {
        window.circuitDesigner = new CircuitDesigner('circuitCanvas');
        
        // Setup component library click handlers
        document.querySelectorAll('.component-item').forEach(item => {
            item.addEventListener('click', () => {
                const type = item.dataset.type;
                if (window.circuitDesigner) {
                    window.circuitDesigner.addComponent(type);
                }
            });
        });
    }
});
