#pragma once
#include "states.h"
#include "map.h"
#include "constants.h"
#include "parlay/random.h"
#include <map>
#include <cstdint>
#include <chrono>

template <class AgentState>
class AgentTemplate
{
public:
  AgentTemplate(int id_, Location &loc_) 
  : loc(&loc_)
  , state(AgentState(id_, loc_, 0.0))
  , rgen(std::chrono::system_clock::now().time_since_epoch().count())
  { }

  Location *find_nearby_task(LocalMapping &local_mapping);
  AgentTransition generate_transition(LocalMapping &local_mapping);
  Direction get_travel_direction(AgentState &new_agent_state);
  bool within_site(int x, int y);
  Location *loc;
  AgentState state;
  parlay::random_generator rgen;
};

using Agent = AgentTemplate<TaskAllocAgentState>;
