# ABM (Agent-Based Model) Project
This project encompasses a compiler, cpu runtime and various XML models for simulation. The compiler is used along with an XML model to produce a library for linking on CPU targets. The cpu runtime executable then attaches to the library to simulate and produce a simulation log which can be viewed by the viewer tool.
A full description of the project can be found in our report here: https://docs.google.com/document/d/1rTIuANcLehnYA-dHvYjS5EHK8xtV4vUDP5kjVS6xR2k/
## Project Layout
The general flow for using the tools provided is as follows:
1. Create an XML file for the model you intend to simulate.
2. Run GAMP, the XML compiler to compile the model into a loadable library.
3. Run PHOBOS, the CPU runtime to simulate the model on a CPU target.
## Build instructions
### XML models
gamp -runtime-path <path-to-runtime-root> <xml-model> -o <output-name>.cpp
g++ -g -c -std=c++14 -o <output-name>.o -fPIC -I<runtime-include-path> <output-name>.cpp
g++ -g -shared -o <model-name>.so <output-name>.o
### GAMP
cd compiler
mkdir build && cd build
cmake ..
make
### Phobos
cd runtime/cpu
mkdir build && cd build
cmake ..
make
### XML Models
Every model to be compiled and simulated needs to have an XML file created which specifies the type of environment within which the simulation takes place and the update rules specific to each agent kind or state.
See https://docs.google.com/document/d/1rTIuANcLehnYA-dHvYjS5EHK8xtV4vUDP5kjVS6xR2k/ for a full description of the XML model language.
#### Boids
Boids is an ABM that is used to model flocks of birds, schools of fish, and herds of animals. In order to model and simulate these complex motions, the boids algorithm follows three basic rules; seperation, cohesion, and alignment. Separation is a boids desire to not “crowd” its neighboring boids. Cohesion represents a boids desire to move towards the average position of their neighboring boids. Alignment motivates the boids to change their current heading to align with the average heading of the neighboring boids. Notice how each rule relies on a local perception of the world and the interactions are confined to a neighborhood. This is an important factor to include in the boids model as it more closely reflects natural systems. In nature, animals do not know the position of all members of their respective group and thus, simulating them as if they do can skew results.
#### Game of Life
Conway's game of life is a cellular based automata which follows a small set of rules; any cell with less than two or more than three neighbors dies, while any within those bounds live. The live cells are the active agents and each simulation iteration the each active agent decides whether or not to become inactive or stay active. Each agent only has to worry about it's immediate neighborhood (as defined in the model's xml file). The logger function for this model simply goes through each active agent and reports its position.
#### Dictyostelium
Our XML models and runtime can be made to simulate an amoebae called dictyostelium. This genus of amoebae will coalesce into groups by releasing a chemoattractant called cAMP and move towards regions with a greater concentration. Similar to how GOL is simulated by coding update rules in the XML model, dictyostelium can be simulated by coding cell update rules and cAMP dispersion into XML.
#### SIR
The SIR model is a subsection of mathematical models used in epidemiology. This model is divided into three important parts: agents that are susceptible, agents that are infectious, and agents that are recovered or resistant. By modeling update rules for each of the three groups, the spreading of various diseases can be simulated.
### Compiler
GAMP, the project compiler takes as input an XML file and produces as output C++ code for linking with the CPU runtime. Other targets other than a CPU will be supported in the future. The main job of the compiler is to take an environment definition and agent rulesets and to compile them down to a basic set of seven 'runtime-calls':
1. New agent
2. Give answer (to another agent)
3. Give neighborhood (to another agent for conditional testing)
4. Receive answer (from another agent)
5. Update agent
6. Log agent
7. Update tick
### Runtime
Currently, only a CPU runtime is available.
#### CPU
The CPU runtime handles the general portions of simulating agent-based models such as environment updates. It links to the 7 runtime-calls listed above in the 'Compiler' section to produce a full simulator for the model in question.
#### FPGA & Clusters
In theory, other targets could be supported by implementing those runtime-calls listed above. Additional code may need to be added for synchronization among other things. Running models on a cluster for example would require changes to the runtime to allow for synchronization of data between compute threads. The compiler would remain relatively static since every portion of code it generates is local to each agent.
### Viewer
For our visualization tool, we used Processing due mainly to its simplicity in getting something on screen. The tool utilized a logging based system which recorded outputs from our runtime execution. In the compiled models the logging function of each agent is implemented and used if a special "log_en" flag is set. The logging function accomodated for the recording of both x and y coordinates, as well as any variables inside an agent's scope. The idea is that at each iteration of the main simulation loop, all variables and agents who had logging enabled would output their values to a log text file. The log file consisted of headers which set what type of simulation to display as well as timestamps of the executed model. In order to display the simulation the visual tool would load in the output log file line by line saving each timestamp. Then the visualizer would be free to go through each timestamp to display the simulation in discrete steps. With multiple log files further statistical analysis can be performed to show the effect of changing inital conditions or varying system runtime parameters.
