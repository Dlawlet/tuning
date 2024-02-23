/**
 * Nodes and edges base class
 */
export class GraphElement {
    isNode() {
        return false;
    }
    setType(type) {
        this.type = type;
        // set default category and parameters of the new type
        this.params = new Map();
        if (this.type.categories.length > 0) {
            this.setCategory(this.type.categories[0]);
        }
        else {
            this.category = undefined;
        }
    }
    getType() {
        return this.type;
    }
    setCategory(category) {
        // check the category is valid for the type of the node
        if (!category || !this.type.categories.includes(category))
            throw new Error("Invalid category");
        this.category = category;
        // set default values for all params in the new category
        this.params = new Map();
        for (const p of category.params) {
            this.params.set(p.id, p.min);
        }
    }
    getCategory() {
        return this.category;
    }
    setParam(id, value) {
        // check the param name is valid
        if (!this.params.has(id))
            throw new TypeError(`${id} is not a valid parameter name for the current type`);
        // check the value is in [min;max]
        const {min, max} = this.category.params.find(p => p.id === id);
        if (value < min || value > max)
            throw new RangeError(`Value ${value} for ${id} is out of range`);
        this.params.set(id, value);
    }
    getParams() {
        return this.params;
    }
    getParamValue(paramId) {
        return this.params.get(paramId);
    }
    setGraph(graph) {
        this.graph = graph;
    }
    move(newposition) {
        this.position = newposition;
    }
    getPosition() {
        return this.position;
    }
}