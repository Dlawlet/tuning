import { GraphEdge } from "../../model/graphEdge";
import { GraphNode } from "../../model/graphNode";
import Graph from "../../model/graph";
import { getNodeTypes, getEdgeTypes } from "../../model/types";

/**
 * Iterator over the string tree, extracting one argument at a time
 */
class CmdLineIterator {
    constructor(cmdlinestring) {
        this.args = cmdlinestring.split(" ");
        this.i = 0;
    }
    next() {
        if (this.i >= this.args.length)
            return "";
        while (this.args[this.i] === "") {
            this.i += 1;
            if (this.i >= this.args.length)
                return "";
        }
        const elem = this.args[this.i];
        this.i += 1;
        return elem;
    }
    end() {
        return this.i >= this.args.length;
    }
}




export class BTreeImporter {
    isStartArg(arg) {
        return arg === "--bt-config";
    }
    isArg(arg) {
        return arg.substring(0, 2) === "--";
    }
    isValue(arg) {
        return !isNaN(arg);
    }
    import(inputString) {
        this.graph = new Graph();
        // build parameters dict
        const iterator = new CmdLineIterator(inputString);
        let dict = {};
        while (!iterator.end()) {
            let key = iterator.next();
            if (this.isStartArg(key) || key === "")
                continue;
            let value = iterator.next();
            if (this.isArg(key) && this.isValue(value))
                dict[key] = value;
            else
                throw "Argument " + key + " is invalid";
        }
        if (inputString !== "") {
            this.importNode(dict, "root");
        }
        return this.graph;
    }
    importNode(dict, nodeID) {
        let argname = "--n" + nodeID;
        if (!dict.hasOwnProperty(argname))
            throw "Cannot find argument " + argname;
        let nodeType = dict[argname];
        let type = getNodeTypes("btree")[Number(nodeType)];
        let node = new GraphNode({ x: 30, y: 30 }, type);
        this.graph.addNode(node);
        this.importParams(dict, nodeID, node);
        if (type.max_outgoing_edges > 0) {
            this.importChildren(dict, nodeID, node);
        }
        return node;
    }
    importChildren(dict, nodeID, parentNode) {
        let argname = "--nchild" + nodeID;
        if (!dict.hasOwnProperty(argname))
            return;
        let childrenNb = dict[argname];
        if (nodeID === "root") {
            nodeID = "";
        }
        for (let i = 0; i < childrenNb; ++i) {
            let node = this.importNode(dict, nodeID + i.toString());
            let edge = new GraphEdge(parentNode, node, getEdgeTypes("btree")[0]);
            if (edge.isValid()) {
                this.graph.addEdge(edge);
            }
        }
    }
    importParams(dict, nodeID, node) {
        const type = node.getType();
        if (type.categories.length > 0) {
            // get category id
            let catargname = "--" + type.categoryid + nodeID;
            if (!dict.hasOwnProperty(catargname))
                return;
            let categoryid = dict[catargname];
            // get category
            const category = type.categories.find(c => c.id === Number(categoryid));
            if (category === undefined)
                return;
            // set node category
            node.setCategory(category);
            // get params
            category.params.forEach(function (p) {
                let paramargname = "--" + p.id + nodeID;
                if (dict.hasOwnProperty(paramargname)) {
                    let value = dict[paramargname];
                    try {
                        node.setParam(p.id, value);
                    } catch (error) {
                        throw "Invalid parameter: " + error.message;
                    }
                }
            });
        }
    }
}