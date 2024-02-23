# AutoMoDe Visualization Tool

This software is a web editor that allows to visualize and edit AutoMoDe behavior trees and finite state machines. These can be used directly in a simulation, and exported in command line or SVG format.

The editor has been tested on Firefox 74 and Chrome 80. Internet Explorer and Edge are not supported.

## Installing and running the editor

The editor is built using npm, which runs on the Node.js Javascript runtime (https://nodejs.org). 
- Type `npm install` in a terminal to install all required dependencies locally. 
- Type `npm start` to start the editor.
- Load `localhost:8080` in your web browser.

The `npm run build` command can be used to get a production minified code, and the `npm run server` to launch a server without automatic code compilation.
For more information on how to use the editor, refer to the manual.

## Running simulations

In order to run a simulation from the editor, you must have an installation of AutoMoDe and the ARGoS3 simulator with the required plugins.

- Create a configuration file named `.env` in this directory with the following content: 
```
AUTOMODE_PATH=path_to_automode_executable
EXPERIMENT_PATH=path_to_argos_experiment_file
```
- The run button in the editor works without additional configuration when it is launched with `npm start`, the output of AutoMoDe can be found in the terminal at the end of the simulation.