import { FSMExporter } from "./fsm/exporter";
import BTreeExporter from "./btree/exporter";

export default function createExporter(mode) {
    switch (mode) {
    case "fsm":
        return new FSMExporter();
    case "btree":
        return new BTreeExporter();
    default:
        throw new Error("Invalid mode");
    }
}