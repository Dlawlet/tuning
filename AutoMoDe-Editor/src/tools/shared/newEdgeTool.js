import { GraphEditorTool } from "../tool";

/**
 * Create a new edge between two nodes
 */
export class GraphEditorNewEdgeTool extends GraphEditorTool {
    onToolSelect() {
        // deselect previous element
        this.graphController.setSelectedElement(undefined);
    }
    getToolId() {
        return "addedge";
    }
    getName() {
        return "Add Edge";
    }
    onMouseDown(pos, element) {
        // select first element
        if (element === undefined) {
            this.graphController.setSelectedElement(undefined);
        }
        // on second element clicked, create edge between them
        else {
            let selected = this.graphController.getSelectedElement();
            if (selected === undefined) {
                this.graphController.setSelectedElement(element);
            }
            else {
                if (element.isNode() && selected.isNode()) {
                    this.graphController.addEdge(selected, element);
                }
                // deleselect
                this.graphController.setSelectedElement(undefined);
            }
        }
    }
}
