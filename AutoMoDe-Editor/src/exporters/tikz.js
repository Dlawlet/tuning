const PX_PER_CM = 1/37.795;
const tikzTemplate = (content) => (
`\\begin{tikzpicture}[every node/.style={minimum size=1cm}]
${content}
\\end{tikzpicture}
`);

const nodeTemplate = (name, shape, decoration, text, position) => (
    `\\node[draw, ${shape}${decoration ? ", " + decoration : ""}, align=center] (${name}) at (${position.x*PX_PER_CM}, -${position.y*PX_PER_CM})
    {${text}};
`);
const lineTemplate = (shape1, shape2) => (
    `\\draw (${shape1}) -- (${shape2});
`);
const arrowTemplate = (shape1, shape2) => (
    `\\draw[->] (${shape1}) -- (${shape2});
`);

export default class TikzExporter {
    export(graph) {
        let content = "";
        let lines = "";
        // export nodes
        const nodes = graph.getNodes()
        for (let i = 0; i < nodes.length; i++) {
            const node = nodes[i];
            const decoration = (i == 0) ? node.type.first_decoration : null;
            content += nodeTemplate(`node${i}`, node.type.shapeName, decoration,
                node.category ? node.category.display_name : node.type.display_text, 
                node.position)
        }
        // export edges
        const edges = graph.getEdges();
        for (let i = 0; i < edges.length; i++) {
            const edge = edges[i];
            const srcNodeIndex = graph.getNodeIndex(edge.srcElement);
            const destNodeIndex = graph.getNodeIndex(edge.destElement);
            if (edge.type.shapeName) {
                content += nodeTemplate(`edge${i}`, edge.type.shapeName, null,
                    edge.category ? edge.category.display_name : edge.type.display_text, 
                    edge.position)
                lines += lineTemplate(`node${srcNodeIndex}`, `edge${i}`)
                    + arrowTemplate(`edge${i}`, `node${destNodeIndex}`);
            } else {
                lines += arrowTemplate(`node${srcNodeIndex}`, `node${destNodeIndex}`);
            }
        }
        return tikzTemplate(content + lines);
    }
}