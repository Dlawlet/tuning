import { h, Fragment } from "preact";
import { getConnectPoint, getDirection, sumPoints, areShapesOverlapping } from "../utils/graphUtils";

export const GraphContainer = ({ elements, tool, svgRef }) => {
    const pointerEventPosition = e => {
        const ctm = svgRef.current.getScreenCTM();
        const x = (e.clientX - ctm.e) / ctm.a;
        const y = (e.clientY - ctm.f) / ctm.d;
        return { x, y };
    };
    const isElementOverlapping = (element) => {
        for (let other of elements.nodes) 
            if ((element !== other) && areShapesOverlapping(element.position, other.position))
                return true;
        for (let other of elements.edges) 
            if ((element !== other) && areShapesOverlapping(element.position, other.position))
                return true;
        return false;
    };
    const handleMouseDown = e => tool.onMouseDown(pointerEventPosition(e), undefined, e.ctrlKey);
    const handleMouseDownOnElement = (event, element, isCtrlKeyPressed) => tool.onMouseDown(pointerEventPosition(event), element, isCtrlKeyPressed);
    const handleMouseUp = e => tool.onMouseUp(pointerEventPosition(e));
    const handleMouseLeave = () => tool.onMouseLeave();
    const handleMouseMove = e => tool.onMouseMove(pointerEventPosition(e));
    return (
        <svg xmlns="http://www.w3.org/2000/svg" ref={svgRef} id="graph" onMouseDown={handleMouseDown} onMouseUp={handleMouseUp} onMouseLeave={handleMouseLeave} onMouseMove={handleMouseMove}>
            <style>
                {style}
            </style>
            <defs>
                <marker id="arrowhead" refX="10" refY="5" markerWidth="10" markerHeight="10" orient="auto-start-reverse">
                    <path d="M 0 0 L 10 5 L 0 10 Z"></path>
                </marker>
            </defs>
            {elements.nodes.map((e, index) => <Node node={e} isSelected={e === elements.selected} isFirst={index === 0} isOverlapping={isElementOverlapping(e)} handleClick={handleMouseDownOnElement} />)}
            {elements.edges.map(e => <Edge edge={e} isSelected={e === elements.selected} isOverlapping={isElementOverlapping(e)} handleClick={handleMouseDownOnElement} />)}
        </svg>
    );
};

const style = `
    svg {
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Ubuntu, Cantarell, "Helvetica Neue", Arial, sans-serif;
	    font-size: 0.9rem;
    }
    .nodeFrame {
        fill: white;
        stroke-width: 1px;
        stroke: black;
        z-index: 10;
    }
    .nodeFrame.selected, .line.selected{
        stroke: #0070ff !important;
    }
    .double {
        stroke-width: 3px;
    }
    .warning {
        stroke: red !important;
    }
    .line {
        stroke: black;
        z-index: 1;
    }
    line.arrow {
        stroke-width: 1px;
        stroke: black;
        z-index: 1;
    }
    .arrowthick {
        stroke-width: 2px;
        stroke: black;
    }
`;

const Node = ({ node, isSelected, isFirst, isOverlapping, handleClick }) => {
    const handleShapeClick = (e) => { e.stopPropagation(); handleClick(e, node, e.ctrlKey); };
    const shape = node.type.shape;
    const decoration = isFirst ? (node.type.first_decoration || "") : "";
    return (
        <Shape className={`${isSelected ? "selected" : ""} ${isOverlapping ? "warning" : ""} ${decoration}`}
            displayTag={shape.display_tag}
            displayOptions={shape.display_opts} position={node.position}
            label={node.category ? node.category.display_name : node.type.display_text}
            handleClick={handleShapeClick} />
    );
};

const Edge = ({ edge, isSelected, isOverlapping, handleClick }) => {
    const handleShapeClick = (e) => { e.stopPropagation(); handleClick(e, edge); };

    // check whether we need to display a shape on the edge
    const shape = edge.type.shape;
    if (shape) {
        const srcPoint = getSrcPoint(edge.srcElement, edge);
        const destPoint = getDestPoint(edge, edge.destElement);
        return (<>
            <line className={`line ${isSelected ? "selected" : ""}`} onMouseDown={handleClick}
                x1={srcPoint.x} y1={srcPoint.y} 
                x2={edge.position.x} y2={edge.position.y} />
            <line className={`line ${isSelected ? "selected" : ""}`} onMouseDown={handleClick}
                marker-end="url(#arrowhead)" x1={edge.position.x} y1={edge.position.y} 
                x2={destPoint.x} y2={destPoint.y} />
            <Shape className={`${isSelected ? "selected" : ""}  ${isOverlapping ? "warning" : ""}`} displayTag={shape.display_tag}
                displayOptions={shape.display_opts} position={edge.position}
                label={edge.category ? edge.category.display_name : edge.type.display_text}
                handleClick={handleShapeClick} />
        </>);
    } else {
        const srcPoint = getSrcPoint(edge.srcElement, edge.destElement);
        const destPoint = getDestPoint(edge.srcElement, edge.destElement);
        return (<>
            <line className={`line ${isSelected ? "selected" : ""}`} onMouseDown={handleClick}
                marker-end="url(#arrowhead)" x1={srcPoint.x} y1={srcPoint.y} 
                x2={destPoint.x} y2={destPoint.y} />
        </>);
    }
};

const Shape = ({ displayTag, displayOptions, position, label, className, handleClick }) => (
    <g transform={`translate(${position.x},${position.y})`} onMouseDown={handleClick}>
        {h(displayTag, { ...displayOptions, class: `nodeFrame ${className}` }, null)}
        <text class="no-select" text-anchor="middle" dominant-baseline="middle" x="0" y="0">
            {label}
        </text>
    </g>
);

const getSrcPoint = (srcElement, destElement) => {
    const srcShape = srcElement.type.shape;
    const direction = getDirection(srcElement.position, destElement.position);
    const connectPoint = getConnectPoint(srcShape.outgoing_connect_type, srcShape.rx, srcShape.ry, direction);
    return sumPoints(srcElement.position, connectPoint);
};
const getDestPoint = (srcElement, destElement) => {
    const destShape = destElement.type.shape;
    const direction = getDirection(destElement.position, srcElement.position);
    const connectPoint = getConnectPoint(destShape.incoming_connect_type, destShape.rx, destShape.ry, direction);
    return sumPoints(destElement.position, connectPoint);
};
