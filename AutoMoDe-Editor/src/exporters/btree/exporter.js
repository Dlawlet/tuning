import { findBTreeRoot } from "../../utils/btreeutils";

export default class BTreeExporter {
    export(graph) {
        const nodes = graph.getNodes();
        // If elements empty, set empty string
        if (nodes.length === 0) {
            return "";
        }
        // Try to find root node
        const root = findBTreeRoot(nodes);
        // build string
        return this.expRootNode(root);
    }
    expRootNode(rootNode) {
        let str = "--bt-config ";
        // add root type
        let type = rootNode.getType().id;
        if (type < 0)
            throw "Invalid configuration : root node type is not selected";
        str += "--nroot " + type + " ";
        str += this.expNodeContent(rootNode, "root");
        return str;
    }
    expNode(node, nodeID) {
        let type = node.getType().id;
        if (type < 0)
            throw "Invalid configuration : nodes types are not selected";
        let str = "--n" + nodeID + " " + type + " ";
        str += this.expNodeContent(node, nodeID);
        return str;
    }
    expNodeContent(node, nodeID) {
        if (node.getOutgoingEdges().length > 0) {
            return this.expNodeChildren(node, nodeID);
        }
        else {
            const type = node.getType();
            const category = node.getCategory();
            if (category) {
                return `--${type.categoryid}${nodeID} ${category.id} ` + this.expNodeParams(node, nodeID);
            } else {
                return this.expNodeParams(node, nodeID);
            }
        }
    }
    expNodeChildren(node, nodeID) {
        let edges = node.getOutgoingEdges();
        let childrenNumber = edges.length;
        let str = "--nchild" + nodeID + " " + childrenNumber + " ";
        // sort child by x position
        edges.sort(function (e1, e2) {
            return e1.getDestNode().getPosition().x > e2.getDestNode().getPosition().x;
        });
        // fix nodeID if root
        if (nodeID === "root")
            nodeID = "";
        // print childs
        for (let i = 0; i < childrenNumber; ++i) {
            str += this.expNode(edges[i].getDestNode(), nodeID + i);
        }
        return str;
    }
    expNodeParams(node, nodeID) {
        let pdict = node.getParams();
        let str = "";
        for (const [key, value] of pdict) {
            str += "--" + key + nodeID + " " + value + " ";
        }
        return str;
    }
}







