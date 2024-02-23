export default class FSMBeautifier {
    constructor(graph) {
        this.graph = graph;
    }
    beautify() {
        // radius of the circle on which the nodes are placed
        const R = 150;
        // coordinates of the center of this circle
        const cx = 500;
        const cy = 300;
        const nodes = this.graph.getNodes();
        const angle = 2*Math.PI/nodes.length;
        // place nodes on the circle
        for (let i = 0; i < nodes.length; i++) {
            nodes[i].move({x: cx - R*Math.cos(i*angle), y: cy - R*Math.sin(i*angle)});
        }
        // place edges, taking multiple transitions into account
        for (let i = 0; i < nodes.length; i++) {
            for (let j = i+1; j < nodes.length; j++) {
                // get edges between node i and j
                let edges = [...nodes[i].getOutgoingEdges(), ...nodes[i].getIncomingEdges()];
                edges = edges.filter(e => (e.getSrcNode() === nodes[j]) || (e.getDestNode() === nodes[j]));
                // compute the middle of the segment [i, j]
                const srcNodePosition = nodes[i].getPosition();
                const destNodePosition = nodes[j].getPosition();
                const middle = {x: (srcNodePosition.x + destNodePosition.x) / 2, y: (srcNodePosition.y + destNodePosition.y) / 2};
                const middleAngle = (i+j) / 2 * angle;
                // slightly move the label if more than one transition
                for (let k = 0; k < edges.length; k++) {
                    const coeff = (k-(edges.length-1)/2)*75;
                    edges[k].move({x: middle.x - coeff * Math.cos(middleAngle), y: middle.y - coeff * Math.sin(middleAngle)});
                }
            }
        }
    }
}