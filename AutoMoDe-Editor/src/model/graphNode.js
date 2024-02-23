import { GraphEdge } from "./graphEdge";
import { GraphElement } from "./graphElement";
/**
 * Nodes of the graph
 */
export class GraphNode extends GraphElement {
    constructor(position, type) {
        super();
        // edges
        this.incomingEdges = [];
        this.outgoingEdges = [];
        // type and parameters
        this.setType(type);
        // graphics
        this.move(position || { x: 0, y: 0 });
    }
    isNode() {
        return true;
    }
    setType(type) {
        super.setType(type);
        // remove edges if there are too many
        while (this.type.max_incoming_edges >= 0 &&
            this.incomingEdges.length > this.type.max_incoming_edges) {
            this.incomingEdges[0].remove();
        }
        while (this.type.max_outgoing_edges >= 0 &&
            this.outgoingEdges.length > this.type.max_outgoing_edges) {
            this.outgoingEdges[0].remove();
        }
    }
    remove() {
        // delete edges before deleting node
        while (this.incomingEdges.length > 0) {
            this.incomingEdges[0].remove();
        }
        while (this.outgoingEdges.length > 0) {
            this.outgoingEdges[0].remove();
        }
        if (this.graph) this.graph.removeNode(this);
    }
    canHaveMoreIncomingEdges() {
        return this.type.max_incoming_edges < 0 ||
            (this.type.max_incoming_edges - this.incomingEdges.length > 0);
    }
    addIncomingEdge(edge) {
        if (edge instanceof GraphEdge && this.canHaveMoreIncomingEdges()) {
            if (!this.incomingEdges.includes(edge)) this.incomingEdges.push(edge);
            return true;
        }
        return false;
    }
    removeIncomingEdge(edge) {
        let index = this.incomingEdges.indexOf(edge);
        if (index > -1)
            this.incomingEdges.splice(index, 1);
    }
    getIncomingEdges() {
        return this.incomingEdges;
    }
    canHaveMoreOutgoingEdges() {
        return this.type.max_outgoing_edges < 0 ||
            (this.type.max_outgoing_edges - this.outgoingEdges.length > 0);
    }
    addOutgoingEdge(edge) {
        if (edge instanceof GraphEdge && this.canHaveMoreOutgoingEdges()) {
            if (!this.outgoingEdges.includes(edge)) this.outgoingEdges.push(edge);
            return true;
        }
        return false;
    }
    removeOutgoingEdge(edge) {
        let index = this.outgoingEdges.indexOf(edge);
        if (index > -1)
            this.outgoingEdges.splice(index, 1);
    }
    getOutgoingEdges() {
        return this.outgoingEdges;
    }
}