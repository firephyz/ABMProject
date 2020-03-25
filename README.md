# ABM (Agent-Based Model) Project

## Build instructions

## Project Layout
### XML Models
#### Boids
Boids is an ABM that is used to model flocks of birds, schools of fish, and herds of animals. In order to model and simulate these
complex motions, the boids algorithm follows three basic rules; seperation, cohesion, and alignment. Separation is a boids desire to not “crowd” its neighboring boids. Cohesion represents a boids desire to move towards the average position of their neighboring boids. Alignment motivates the boids to change their current heading to align with the average heading of the neighboring boids. Notice how each rule relies on a local perception of the world and the interactions are confined to a neighborhood. This is an important factor to include in the boids model as it more closely reflects natural systems. In nature, animals do not know the position of all members of their respective group and thus, simulating them as if they do can skew results. 
#### Game of Life
Conway's game of life is a cellular based automata which follows a small set of rules; any cell with less than two or more than three neighbors dies, while any within those bounds live. The live cells are the active agents and each simulation iteration the each active agent decides whether or not to become inactive or stay active. Each agent only has to worry about it's immediate neighborhood (as defined in the model's xml file). The logger function for this model simply goes through each active agent and reports its position. 
#### Granuloma
#### SIR
### Compiler
### Runtime
#### CPU
#### FPGA & Clusters
### Viewer
For our visualization tool, we used Processing due mainly to its simplicity in getting something on screen. The tool utilized a logging based system which recorded outputs from our runtime execution. In the compiled models the logging function of each agent is implemented and used if a special "log_en" flag is set. The logging function accomodated for the recording of both x and y coordinates, as well as any variables inside an agent's scope. The idea is that at each iteration of the main simulation loop, all variables and agents who had logging enabled would output their values to a log text file. The log file consisted of headers which set what type of simulation to display as well as timestamps of the executed model. In order to display the simulation the visual tool would load in the output log file line by line saving each timestamp. Then the visualizer would be free to go through each timestamp to display the simulation in discrete steps. With multiple log files further statistical analysis can be performed to show the effect of changing inital conditions or varying system runtime parameters. 
