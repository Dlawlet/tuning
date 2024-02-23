/**
 * Utility functions for graphs
 */
const OVERLAP_DISTANCE = 50;

/**
 * Sum two points
 */
export function sumPoints(p1, p2) {
    const p = {
        x:(parseInt(p1.x) + parseInt(p2.x)),
        y:(parseInt(p1.y) + parseInt(p2.y))
    };
    return p;
}

/**
 * Subtract two points
 */
export function subtractPoints(p1, p2) {
    const p = {
        x:(parseInt(p1.x) - parseInt(p2.x)),
        y:(parseInt(p1.y) - parseInt(p2.y))
    };
    return p;
}

/**
 * Return the middle of a segment
 */
export function middle(p1, p2) {
    return {x: (p1.x+p2.x) / 2, y: (p1.y+p2.y) / 2};
}

export function getDirection(p1, p2) {
    let x = p2.x - p1.x;
    let y = p2.y - p1.y;
    let norm = Math.sqrt(x**2 + y**2);
    x /= norm;
    y /= norm;
    return {x, y};
}

export function getConnectPoint(connectType, rx, ry, direction) {
    switch (connectType) {
    case "N":
        return {x: 0, y: -ry};
    case "S":
        return {x: 0, y: ry};
    case "E":
        return {x: rx, y: 0};
    case "W":
        return {x: -rx, y: 0};
    case "oval":
        return {x: rx*direction.x, y: ry*direction.y};
    default:
        return {x: 0, y: 0};
    }
}
/**
 * Check whether shapes are overlapping
 * @param {position of the first shape} p1 
 * @param {position of the second shape} p2 
 */
export function areShapesOverlapping(p1, p2) {
    return ((p1.x - p2.x)**2 + (p1.y - p2.y)**2) < OVERLAP_DISTANCE**2;
}