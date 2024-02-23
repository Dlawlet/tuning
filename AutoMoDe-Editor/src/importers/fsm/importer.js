import { GraphEdge } from "../../model/graphEdge";
import { GraphNode } from "../../model/graphNode";
import { FSMParser } from "./fsmparser";
import Graph from "../../model/graph";
import { getNodeTypes, getEdgeTypes } from "../../model/types";

export class FSMImporter {
    import(input) {
        const parser = new FSMParser(input);
        this.states = parser.parse();
        this.nstates = this.states.length;
        this.graph = new Graph();
        for (const state of this.states) {
            this.importNode(state);
        }
        this.importEdges();
        return this.graph;
    }
    importNode(state) {
        // define node type (only 1 possibility, they are all states)
        const type = getNodeTypes("fsm")[0];
        // define the position of the node
        const position = { x: 100 + (state.index % 2) * 200, y: 100 + Math.floor(state.index / 2) * 200 };
        const node = new GraphNode(position, type);
        this.importNodeParams(state, node);
        this.graph.addNode(node);
    }
    importNodeParams(state, node) {
        const type = node.getType();
        if (type.categories.length > 0) {
            // set node category
            const category = type.categories.find(c => c.id === state.behaviour);
            if (category === undefined)
                throw "Invalid behaviour number: " + state.behaviour;
            node.setCategory(category);
            // set params
            for (const [pname, pvalue] of state.params) {
                try {
                    node.setParam(pname, pvalue);
                } catch (error) {
                    throw "Invalid parameter: " + error.message;
                }
            }
        }
    }
    importEdges() {
        for (let i = 0; i < this.states.length; i++) {
            const state = this.states[i];
            for (const transition of state.transitions) {
                this.importEdge(i, transition);
            }
        }
    }
    importEdge(stateIndex, transition) {
        if (transition.endState < 0 || transition.endState >= this.nstates)
            throw `Invalid transition for state ${stateIndex}`;
        const nodes = this.graph.getNodes();
        const startNode = nodes[stateIndex];
        const destNode = nodes[transition.endState];
        const edge = new GraphEdge(startNode, destNode, getEdgeTypes("fsm")[0]);
        this.importEdgeParams(edge, transition);
        if (edge.isValid()) {
            this.graph.addEdge(edge);
        }
    }
    importEdgeParams(edge, transition) {
        const type = edge.getType();
        if (type.categories.length > 0) {
            // set edge category
            const category = type.categories.find(c => c.id === transition.condition);
            if (category === undefined)
                throw "Invalid behaviour number: " + transition.condition;
            edge.setCategory(category);
            // set params
            for (const [pname, pvalue] of transition.params) {
                try {
                    edge.setParam(pname, pvalue);
                } catch (error) {
                    throw "Invalid parameter: " + error.message;
                }
            }
        }
    }
}