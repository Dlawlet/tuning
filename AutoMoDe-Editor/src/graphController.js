import { GraphNode } from "./model/graphNode";
import { GraphEdge } from "./model/graphEdge";
import Graph from "./model/graph";
import createExporter from "./exporters";
import createImporter from "./importers";
import createBeautifier from "./model/beautifiers";
import {getNodeTypes, getEdgeTypes} from "./model/types";
import TikzExporter from "./exporters/tikz";
import { exporttofile } from "./utils/cmdlineUtils";

export default class GraphController {
    constructor() {
        this.observers = [];
        // graph
        this.graph = new Graph();
        this.selectedElement = undefined;
        //mode
        this.setMode("fsm");
        this.updateGraph();
    }
    addObserver(observer) {
        this.observers.push(observer);
    }
    removeObserver(observer) {
        let index = this.observers.indexOf(observer);
        if (index !== -1) this.observers.splice(index, 1);
    }
    notify(event) {
        for (const l of this.observers) {
            const handler = l[event];
            if (typeof(handler) === "function")
                handler();
        }
    }
    getNodeTypes() {
        return getNodeTypes(this.mode);
    }
    getEdgeTypes() {
        return getEdgeTypes(this.mode);
    }
    addNode(position) {
        const node = new GraphNode(position, 
            getNodeTypes(this.mode)[0]);
        this.graph.addNode(node);
        this.callExporter();
        this.setSelectedElement(node);
    }
    addEdge(srcElement, destElement) {
        // create edge
        const edge = new GraphEdge(srcElement, destElement, 
            getEdgeTypes(this.mode)[0]);
        // if edge valid, add it
        if (edge.isValid()) {
            this.graph.addEdge(edge);
            this.callExporter();
            this.setSelectedElement(edge);
        }
    }
    setFirstElement(newFirst) {
        this.graph.setFirstNode(newFirst);
        this.callExporter();
        this.updateGraph();
    }
    removeElement(element) {
        if (element instanceof GraphNode) {
            this.graph.removeNode(element);
        } else if (element instanceof GraphEdge) {
            this.graph.removeEdge(element);
        }
        if (this.selectedElement === element)
            this.setSelectedElement(undefined);
        this.callExporter();
    }
    getElements() {
        return {nodes: this.graph.getNodes(), edges: this.graph.getEdges(), 
            selected: this.selectedElement};
    }
    getCmdline() {
        return this.cmdline;
    }
    getErrorMessage() {
        return this.errorMessage;
    }
    setCmdline(cmdline, errorMessage) {
        this.cmdline = cmdline;
        this.errorMessage = errorMessage;
        this.notify("cmdlineChange");
    }
    clearElements() {
        this.graph = new Graph();
        this.callExporter();
        this.setSelectedElement();
    }
    setSelectedElement(element) {
        this.selectedElement = element;
        this.updateGraph();
    }
    getSelectedElement() {
        return this.selectedElement;
    }
    setSelectionType(typeId) {
        const element = this.getSelectedElement();
        if (element.isNode()) {
            element.setType(this.getNodeTypes().find(t => t.id === typeId));
        } else {
            element.setType(this.getEdgeTypes().find(t => t.id === typeId));
        }
        this.callExporter();
        this.updateGraph();
    }
    setSelectionCategory(categoryId) {
        const element = this.getSelectedElement();
        const type = this.selectedElement.getType();
        if (type.categoryid !== undefined && categoryId !== undefined)
            element.setCategory(type.categories.find(c => c.id === categoryId));
        this.callExporter();
    }
    setSelectionParam(paramId, value) {
        const element = this.getSelectedElement();
        element.setParam(paramId, value);
        this.callExporter();
    }
    callExporter() {
        const exporter = createExporter(this.mode);
        try {
            this.setCmdline(exporter.export(this.graph));
        } catch (err) {
            if (typeof err === "string")
                this.setCmdline("", err);
            else
                console.log(err);
        }
    }
    exportToTikz() {
        const exporter = new TikzExporter();
        const result = exporter.export(this.graph);
        exporttofile(result, "text", "export.tikz");
    }
    beautifyGraph() {
        const beautifier = createBeautifier(this.mode, this.graph);
        beautifier.beautify();
        this.updateGraph();
    }
    updateGraph() {
        this.notify("elementsChange");
    }
    callImporter(cmdlineString) {
        const importer = createImporter(this.mode);
        try {
            const newGraph = importer.import(cmdlineString);
            this.clearElements();
            this.graph = newGraph;
            this.beautifyGraph();
            // clean cmdline
            this.callExporter();
        } catch (err) {
            if (typeof err === "string")
                this.setCmdline(cmdlineString, err);
            else 
                console.log(err);
        }
    }
    getMode() {
        return this.mode;
    }
    setMode(mode) {
        if (this.mode === mode) return;
        switch (mode) {
        case "fsm":
            this.mode = "fsm";
            break;
        case "btree":
            this.mode = "btree";
            break;
        default:
            throw new Error("Wrong mode selected");
        }
        this.clearElements();
        this.notify("modeChange");
    }
}