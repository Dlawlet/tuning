import FSMBeautifier from "./fsm/beautifier";
import { BTreeBeautifyTool } from "./btree/beautifier";

export default function createBeautifier(mode, graph) {
    switch (mode) {
    case "fsm": return new FSMBeautifier(graph);
    case "btree": return new BTreeBeautifyTool(graph);
    default: throw new Error("Invalid mode");
    }
}