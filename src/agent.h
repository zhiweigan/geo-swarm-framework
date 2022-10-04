#pragma once
#include "states.h"
#include "map.h"
#include <cstdint>

class Agent 
{
public:
  Agent(int16_t id_, Location &loc_) 
  : loc(&loc_)
  , state(AgentState(id_, loc_, 0.0))
  { }
  
private:
  Location *loc;
  AgentState state;
};

