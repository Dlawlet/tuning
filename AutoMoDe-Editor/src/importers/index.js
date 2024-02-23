import { FSMImporter } from "./fsm/importer";
import { BTreeImporter } from "./btree/importer";

export default function createImporter(mode) {
    switch (mode) {
    case "fsm": return new FSMImporter();
    case "btree": return new BTreeImporter();
    default: throw new Error("Invalid mode");
    }
}