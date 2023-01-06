# Geo Swarm Framework

Introduction: TODO

## How to: Build & Run
```
cd build/
cmake .. -DAPP=density
make
./density
```

## Application

Each application has to contain the following files. Refer to an implemented application for an example.

### states.h
This file defines the agent and location states that are specific to this application, as well as the structure of a message that could be sent. 

### agents.h
This file contains the implementation for Agent objects. 

### config.cpp
This file implements the unimplemented functions for the configuration.h file. This determines how each agent interacts with the configuration.

### constants.h
This file contains key constants required for the project.

### simulate.cpp
This file sets up and runs the simulation.

