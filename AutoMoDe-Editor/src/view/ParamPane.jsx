import {h, Fragment} from "preact";

export const ParamPane = ({ element, graphController }) => {
    if (!element)
        return <aside class="right-pane" />;
    //model select
    const types = element.isNode()
        ? graphController.getNodeTypes()
        : graphController.getEdgeTypes();
    // parameter elements
    const type = element.getType();
    const category = element.getCategory();
    // event handlers
    const setElementType = (typeId) => graphController.setSelectionType(typeId);
    const setElementCategory = (categoryId) => graphController.setSelectionCategory(categoryId);
    const setElementParam = (paramId, value) => graphController.setSelectionParam(paramId, value);
    return (
        <aside class="right-pane">
            {types.length > 1 &&
                <Select options={types} selectedOption={type} 
                    onOptionChange={setElementType} />
            }
            {category && <>
                {type.categoriesName}
                <Select options={type.categories} selectedOption={category} 
                    onOptionChange={setElementCategory} />
                {category.params.map(p => (<label>
                    {p.name}
                    <ParamInput param={p} value={element.getParamValue(p.id)} 
                        onParamChange={setElementParam} />
                </label>))}
            </>}
        </aside>
    );
};

const Select = ({ options, selectedOption, onOptionChange }) => {
    const handleChange = e => onOptionChange(Number(e.target.value));
    return (<select class="paraminput" onChange={handleChange}>
        {options.map(o => (<option value={o.id} selected={o === selectedOption}>
            {o.name}
        </option>))}
    </select>);
};

const ParamInput = ({ param, value, onParamChange }) => {
    const handleChange = e => onParamChange(param.id, Number(e.target.value));
    return (<div class="d-flex">
        <input class="paraminput d-block" type="number" name={param.name} onChange={handleChange} min={param.min} max={param.max} step={param.step} value={value} />
        <span class="textrange">[{param.min};{param.max}]</span>
    </div>);
};
