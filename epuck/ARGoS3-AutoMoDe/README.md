# README
ARGoS3-AutoMoDe
=====================

All useful information about the ARGoS3-AutoMoDe package, including
installation and utilization instructions, are regrouped in the
following technical report ([techrep](#bibliography)). Please cite this report if
you use the ARGoS3-AutoMoDe package.

## Package content

- `bin` This empty folder will contain the executable automode_main
after compilation.
- `experiments` The folder where all experiments configuration files
  (.argos or .xml) are located
    - `chocolate` The experiment configuration files of the
        experiments described in [chocolate](#bibliography)
    - `example` The experiment configuration file used as example in
        the technical report of the package.
- `optimization` The folder where one should place files regarding
    the optimization algorithm used (including grammar generator for
    the irace algorithm)
    - `example` The necessary elements to launch irace on a small example described in the technical report.
- `src` The source files of ARGoS3-AutoMoDe package.
    - `cmake` The .cmake files for ARGoS3.
    - `core` The core classes of ARGoS3-AutoMoDe.
    - `modules` The behaviors and conditions modules described in [chocolate](#bibliography)
- `AutoMoDeMain.cpp` :: The main procedure to launch ARGoS3-AutoMoDe.


## Installation
### Dependencies:
- [ARGoS3](https://github.com/ilpincy/argos3) (3.0.0-beta48)
- [argos3-epuck](https://github.com/demiurge-project/argos3-epuck) (v48)
- [experiments-loop-functions](https://github.com/demiurge-project/experiments-loop-functions) (master)
- [demiurge-epuck-dao](https://github.com/demiurge-project/demiurge-epuck-dao) (master)

### Compiling AutoMoDe:
    $ git clone https://github.com/demiurge-project/ARGoS3-AutoMoDe.git
    $ cd argos3-AutoMoDe
    $ mkdir build
    $ cmake ..
    $ make

Once compiled, the `bin/` folder should contain the `automode_main`
executable.

## How to use
### Run a single experiment:
To run a single experiment, you need to use the `automode_main`
as follows:

    usage: automode_main [-r | --readable] [-c | --config-file configfile] [-m | --method method] [-s | --seed seed] [--fsm-config fsmconfig]
        options:
            [-c | --config-file configfile] Path to the .argos|.xml experiment file to use [REQUIRED]
            [-r | --readable] Prints an URL containing a DOT representation of the finite state machine [OPTIONAL]
            [-s | --seed seed] The random seed for the ARGoS3 simulator [OPTIONAL]
            [--fsm-config fsmconfig] the description of the finite state machine in the AutoMoDe format [REQUIRED]


### Run the design process
To run the design process you will need the
[`irace` package](http://iridia.ulb.ac.be/irace/) and a
fonctional `R` distribution.

If you are using an MPI capable computing cluster you can use the
`parallel-irace-mpi` located in the `optimization/` folder.
Otherwise, you can directly launch irace with the command:

    irace --exec-dir=EXECDIR --seed SEED --scenario scenario
        options:
            [--exec-dir=execdir] the execution folder
            [--seed seed] the random seed for the optimization algorithm
            [--scenario scenario] the scenario file of the experiment

For example (in the `optimization/` folder):

    irace --exec-dir=aggregation --seed 1234 --scenario scenario.txt

This will run the optimization algorithm for 200k simulation in order
to train for the aggregation mission (see [chocolate](#bibliography) for details
on missions).



## References
### Bibliography

- [techrep] Ligot, A., Hasselmann, K., Delhaisse, B., Garattoni, L., Francesca, G., & Birattari, M. (2017). AutoMoDe, NEAT, and EvoStick: implementations for the E-puck robot in ARGoS3. Technical report TR/IRIDIA/2017-002, IRIDIA, Université libre de Bruxelles, Belgium.
- [chocolate] Francesca, G., Brambilla, M., Brutschy, A., Garattoni, L., Miletitch, R., Podevijn, G., ... & Mascia, F. (2015). AutoMoDe-Chocolate: automatic design of control software for robot swarms. Swarm Intelligence, 9(2-3), 125-152.
