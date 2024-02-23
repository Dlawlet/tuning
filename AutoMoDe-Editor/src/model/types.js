import btreeNodeTypes from "Config/btree/nodeTypes.json";
import btreeEdgeTypes from "Config/btree/edgeTypes.json";
import btreeNodeCategories from "Config/btree/nodeCategories.json";
import btreeEdgeCategories from "Config/btree/edgeCategories.json";
import btreeShapes from "Config/btree/shapes.json";
import fsmNodeTypes from "Config/fsm/nodeTypes.json";
import fsmEdgeTypes from "Config/fsm/edgeTypes.json";
import fsmNodeCategories from "Config/fsm/nodeCategories.json";
import fsmEdgeCategories from "Config/fsm/edgeCategories.json";
import fsmShapes from "Config/fsm/shapes.json";

const lFsmNodeTypes = loadTypes(fsmNodeTypes, fsmNodeCategories, fsmShapes);
const lFsmEdgeTypes = loadTypes(fsmEdgeTypes, fsmEdgeCategories, fsmShapes);
const lBtreeNodeTypes = loadTypes(btreeNodeTypes, btreeNodeCategories, btreeShapes);
const lBtreeEdgeTypes = loadTypes(btreeEdgeTypes, btreeEdgeCategories, btreeShapes);

export function getNodeTypes(mode) {
    switch(mode) {
    case "fsm": return lFsmNodeTypes;
    case "btree": return lBtreeNodeTypes;
    default: throw new Error("Invalid mode");
    }
}
export function getEdgeTypes(mode) {
    switch(mode) {
    case "fsm": return lFsmEdgeTypes;
    case "btree": return lBtreeEdgeTypes;
    default: throw new Error("Invalid mode");
    }
}

// load types, categories and shapes from json config files
function loadTypes(types, categories, shapes) {
    const lTypes = [...types];
    for (const type of lTypes) {
        const typeCategories = categories.find(category => category.typeid === type.id);
        if (typeCategories) {
            type.categories = typeCategories.categories;
            type.categoryid = typeCategories.categoryid;
            type.categoriesName = typeCategories.categories_name;
        } else {
            type.categories = [];
        }
        type.shapeName = type.shape;
        type.shape = shapes[type.shape];
    }
    return lTypes;
}