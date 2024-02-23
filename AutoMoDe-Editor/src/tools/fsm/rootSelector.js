import { GraphEditorTool } from "../tool";

/**
 * Delete element
 */
export class GraphEditorRootSelectorTool extends GraphEditorTool {
    getName() {
        return "Set initial state";
    }
    onMouseDown(pos, element) {
        if (element !== undefined && element.isNode())
            this.graphController.setFirstElement(element);
    }
}