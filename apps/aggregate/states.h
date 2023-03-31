#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include "../template_states.h"

struct AggregateLocationState
{
  AggregateLocationState(
    Position location_
  )
    : location(location_)
  { }
  Position location;
};

struct AggregateAgentState
{
  AggregateAgentState(int id_) 
  : id(id_)
  , responsiveness(0)
  { }

  int id;
  double responsiveness;
};

struct AggregateAgentMessage
{

};

using Location                = LocationTemplate<AggregateLocationState>;
using AgentTransition         = AgentTransitionTemplate<AggregateLocationState, AggregateAgentState>;
using AgentMessage            = AggregateAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage>*>>;
