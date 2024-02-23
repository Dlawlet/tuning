import {h, Fragment} from "preact";
import {useState, useEffect, useRef} from "preact/hooks";
import { ToolPane } from "./ToolPane.jsx";
import { Header } from "./Header.jsx";
import { ParamPane } from "./ParamPane.jsx";
import { GraphContainer } from "./GraphContainer.jsx";
import { ErrorBar } from "./ErrorBar.jsx";

export default ({graphController, tools}) => {
    // UI state
    const [elements, setElements] = useState(graphController.getElements());
    const [mode, setMode] = useState(graphController.getMode());
    const [selectedTool, setSelectedTool] = useState(tools[mode][0]);
    const [cmdline, setCmdline] = useState(graphController.getCmdline());
    const [errorMessage, setErrorMessage] = useState(graphController.getErrorMessage());
    // reference to the graph svg
    const svgRef = useRef(null);

    const changeMode = (newMode) => {
        setMode(newMode);
        // reset the UI
        setElements(graphController.getElements());
        setSelectedTool(tools[newMode][0]);
    };
    const changeCmdline = (newCmdline, newErrorMessage) => {
        setCmdline(newCmdline);
        setErrorMessage(newErrorMessage);
    };
    const changeTool = (newTool) => {
        if (selectedTool !== newTool) {
            setSelectedTool(newTool);
            graphController.setSelectedElement();
        }
    };
    // listen to changes to the model
    useEffect(() => {
        const observer = {
            elementsChange: () => setElements(graphController.getElements()),
            modeChange: () => changeMode(graphController.getMode()),
            cmdlineChange: () => changeCmdline(graphController.getCmdline(), graphController.getErrorMessage())
        };
        graphController.addObserver(observer);
        return () => {
            graphController.removeObserver(observer);
        };
    }, [graphController]);

    return (
        <>
            <Header graphController={graphController} cmdline={cmdline} />
            <ErrorBar message={errorMessage} />
            <article class="graph-container">
                <GraphContainer elements={elements} tool={selectedTool}
                    svgRef={svgRef} />
            </article>

            <ToolPane graphController={graphController}
                tools={tools[mode]}
                selectedTool={selectedTool}
                setSelectedTool={changeTool}
                svgRef={svgRef} />
            <ParamPane element={elements.selected} graphController={graphController} />
        </>
    );  
};