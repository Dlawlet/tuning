import { findBTreeRoot } from "../../../utils/btreeutils";

/**
 * Beautifier algorithm
 */

/**
 * Move nodes to make the tree looks better
 */
export function beautifyBTree(rootNode) {

    let area = {x:0, y:0, w:1000, h:500, relh:0, relw:0};

    let rootInfo = getBTreeNodeInfo(rootNode);

    setBTreeNodePosition(rootNode, rootInfo, area);
}

/**
 * Recursively collect node informations
 */
function getBTreeNodeInfo(node) {

    let info = {depth:1, cumulativeChildrenNb:0, childrenInfo:[]};
    let edges = node.getOutgoingEdges();

    if(edges.length === 0) {
        info.cumulativeChildrenNb = 1;
    }

    edges.forEach(function(e) {
        let i = getBTreeNodeInfo(e.getDestNode());
        if(info.depth < i.depth + 1) {
            info.depth = i.depth + 1;
        }
        info.cumulativeChildrenNb += i.cumulativeChildrenNb;
        info.childrenInfo.push(i);
    });

    return info;
}

/**
 * Recursively update node positions
 */
function setBTreeNodePosition(node, info, area) {

    if(area.relh === 0)
        area.relh = area.h / info.depth;
    if(area.relw === 0)
        area.relw = area.w / info.cumulativeChildrenNb;

    node.move({x:(area.x + area.w/2), y:(area.y + area.relh/2)});

    let edges = node.getOutgoingEdges();
    let widthAccumulator = 0;

    for(let i = 0; i < edges.length; ++i) {
        let n = edges[i].getDestNode();
        let ci = info.childrenInfo[i];

        let relwidth = area.relw * ci.cumulativeChildrenNb;

        let a = {x:area.x + widthAccumulator,
            y:area.y + area.relh,
            w:relwidth,
            h:area.h - area.relh,
            relw:area.relw,
            relh:area.relh};

        setBTreeNodePosition(n, ci, a);

        widthAccumulator += relwidth;
    }
}


/**
 * Beautify tool
 */
export class BTreeBeautifyTool {
    constructor(graph) {
        this.graph = graph;
    }
    beautify() {
    // call beautifier algo
        try {
            let root = findBTreeRoot(this.graph.getNodes());
            beautifyBTree(root);
        }
        catch (err) {
            alert(err);
        }
    }
}


