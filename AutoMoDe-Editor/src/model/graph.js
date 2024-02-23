export default class Graph {
    constructor() {
        this.nodes = [];
        this.edges = [];
    }
    getNodes() {
        return this.nodes;
    }
    getEdges() {
        return this.edges;
    }
    getNodeIndex(node) {
        let index = this.nodes.indexOf(node);
        if (index === -1) throw new Error("Node not in list");
        return index;
    }
    addNode(node) {
        node.setGraph(this);
        this.nodes.push(node);
    }
    addEdge(edge) {
        edge.setGraph(this);
        this.edges.push(edge);
    }
    removeNode(node) {
        let index = this.nodes.indexOf(node);
        if (index > -1) {
            this.nodes.splice(index, 1);
            node.remove();
        }
    }
    removeEdge(edge) {
        let index = this.edges.indexOf(edge);
        if (index > -1) {
            this.edges.splice(index, 1);
            edge.remove();
        }
    }
    setFirstNode(newFirst) {
        let index;
        if (newFirst && (index = this.nodes.indexOf(newFirst)) > -1) {
            [this.nodes[0], this.nodes[index]] = [this.nodes[index], this.nodes[0]];
        }
    }
}