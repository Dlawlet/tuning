import { GraphEditorTool } from "../tool";

/**
 * Delete element
 */
export class GraphEditorDeleteTool extends GraphEditorTool {
    getName() {
        return "Delete";
    }
    onMouseDown(pos, element) {
        // remove cliked element
        if (element !== undefined) {
            this.graphController.removeElement(element);
        }
    }
}
