
export class FSMExporter{
    export(graph) {
        this.nodes = graph.getNodes();
        this.edges = graph.getEdges();
        // calculate the number of states
        this.nstates = this.nodes.length;
        // if there are no element, return the empty string
        if (this.nstates === 0)
            return "";
        // export elements
        this.result = "";
        this.exportElements();
        return this.result;
    }
    exportElements() {
        this.result += "--fsm-config --nstates " + this.nstates + " ";
        // for each node, export its behaviour, params and transitions
        for (let i = 0; i < this.nodes.length; i++) {
            const elem = this.nodes[i];
            if (elem.isNode()) {
                this.result += `--s${i} ${elem.category.id} `;
                this.exportNodeParams(elem, i);
                this.exportNodeTransitions(elem, i);
            }
        }
    }
    exportNodeParams(node, nodeIndex) {
        const params = node.getParams();
        for (const [p, value] of params) {
            this.result += `--${p}${nodeIndex} ${value} `;
        }
    }
    exportNodeTransitions(node, nodeIndex) {
        if (node.getOutgoingEdges().length > 0) {
            // export transitions count
            this.result += "--n" + nodeIndex + " " + node.getOutgoingEdges().length + " ";
            // export transitions
            for (let j = 0; j < node.getOutgoingEdges().length; j++) {
                const transition = node.getOutgoingEdges()[j];
                // find index of transition target
                let targetIndex = this.getNodeIndex(transition.getDestNode());
                if (targetIndex > nodeIndex)
                    targetIndex--;
                else if (targetIndex === nodeIndex)
                    throw new Error("A node cannot have a transition to itself");
                // export transition target, condition and params
                this.result += "--n" + nodeIndex + "x" + j + " " + targetIndex + " ";
                this.result += "--c" + nodeIndex + "x" + j + " " + transition.category.id + " ";
                this.exportEdgeParams(transition, nodeIndex, j);
            }
        }
    }
    exportEdgeParams(edge, startNodeIndex, edgeIndex) {
        const params = edge.getParams();
        for (const [p, value] of params) {
            this.result += "--" + p + startNodeIndex + "x" + edgeIndex + " " + value + " ";
        }
    }
    getNodeIndex(node) {
        let index = this.nodes.indexOf(node);
        if (index === -1) throw new Error("Node not in list");
        return index;
    }
}