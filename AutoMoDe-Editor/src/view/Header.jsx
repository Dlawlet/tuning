import {h, Fragment, createRef} from "preact";
import * as CmdlineUtils from "../utils/cmdlineUtils";

export const Header = ({graphController, cmdline}) => {
    const mode = graphController.getMode();
    const setMode = mode => graphController.setMode(mode);

    return (
        <header>
            <div id="title-container">
                <button className={mode === "fsm" ? "selected" : ""}
                    onClick={() => setMode("fsm")}>FSM</button>
                <button className={mode === "btree" ? "selected" : ""}
                    onClick={() => setMode("btree")}>BTree</button>
            </div>
            <Cmdline graphController={graphController} cmdline={cmdline} />
        </header>
    );
};

const Cmdline = ({graphController, cmdline}) => {
    const cmdlineInput = createRef();
    const openFileInput = createRef();

    const openFile = async e => {
        const cmdline = await CmdlineUtils.importfromfile(e.target.files[0]);
        graphController.callImporter(cmdline);
    };
    const triggerOpenDialog = () => openFileInput.current.click();
    const importCmdline = () => graphController.callImporter(cmdline);
    const copyCmdline = () => {
        cmdlineInput.current.select();
        document.execCommand("copy");
    };
    const saveCmdline = () => CmdlineUtils.exporttofile(cmdline, "text/plain", "cmdline.txt");
    const executeCmdline = () => CmdlineUtils.execinsimulator(cmdline);

    return (
        <div class="cmdline-container">
            <input ref={openFileInput} type="file" onChange={openFile} style="display: none;" />
            <button class="left" onClick={triggerOpenDialog}>
                <span class="underline">O</span>pen
            </button>
            <input ref={cmdlineInput} type="text" value={cmdline} onChange={importCmdline}
                onInput={e => graphController.setCmdline(e.target.value)}
                placeholder="Command line string" />
            <button class="right" onClick={copyCmdline}>
                <span class="underline">C</span>opy
            </button>
            <button class="right" onClick={saveCmdline}>
                <span class="underline">S</span>ave
            </button>
            <button class="right" onClick={executeCmdline}>
                E<span class="underline">x</span>ec
            </button>
        </div>
    );
};
