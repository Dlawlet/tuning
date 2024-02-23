import {h, Fragment} from "preact";
import { exporttofile } from "../utils/cmdlineUtils";

export const ToolPane = ({ graphController, tools, selectedTool, setSelectedTool, svgRef }) => {
    const exportSvg = () => {
        const bbox = svgRef.current.getBBox();
        const svg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="${bbox.x-10} ${bbox.y-10} ${bbox.width+20} ${bbox.height+20}">` 
            + svgRef.current.innerHTML + "</svg>";
        exporttofile(svg, "image/svg+xml", "export.svg");
    };
    return (
        <div class="left-pane">
            <div id="tools-container">
                {tools.map(tool => 
                    <button className={tool === selectedTool ? "tool selected" : "tool"} onClick={() => setSelectedTool(tool)}>
                        {tool.getName()}
                    </button>
                )}
            </div>
            <hr />
            <button className="button" onClick={() => graphController.beautifyGraph()}>
                Beautify
            </button>
            <hr />
            <button className="button" onClick={exportSvg}>
                SVG Export
            </button>
            <button className="button" onClick={() => graphController.exportToTikz()}>
                Tikz Export
            </button>
        </div>
)};
