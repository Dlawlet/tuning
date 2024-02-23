import {h, Fragment} from "preact";

export const ErrorBar = ({message}) => (
    <>
        {message && 
            <div class="error-bar">
                {message}
            </div>
        }
    </>
);