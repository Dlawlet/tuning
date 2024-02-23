import { GraphElement } from "./graphElement";

/**
 * Bind two nodes ('src' and 'dest')
 */
export class GraphEdge extends GraphElement {
    constructor(srcElement, destElement, type) {
        super();
        // model and parameters
        this.setType(type);
        // bind src and dest
        if (srcElement.canHaveMoreOutgoingEdges() && destElement.canHaveMoreIncomingEdges()) {
            this.srcElement = srcElement;
            this.destElement = destElement;
            this.srcElement.addOutgoingEdge(this);
            this.destElement.addIncomingEdge(this);
            // set default position
            this.move({x: (this.srcElement.getPosition().x + this.destElement.getPosition().x)/2,
                y: (this.srcElement.getPosition().y + this.destElement.getPosition().y)/2});
        }
    }
    isValid() {
        return this.srcElement !== undefined && this.destElement !== undefined;
    }
    remove() {
        this.srcElement.removeOutgoingEdge(this);
        this.destElement.removeIncomingEdge(this);
        if (this.graph) this.graph.removeEdge(this);
    }
    getSrcNode() {
        return this.srcElement;
    }
    getDestNode() {
        return this.destElement;
    }
}