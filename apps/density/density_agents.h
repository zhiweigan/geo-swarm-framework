#pragma once
#include "density_states.h"
#include "../../src/map.h"
#include "../../src/constants.h"
#include "../../src/parlay/random.h"
#include "../../src/geo_utils.h"
#include <map>
#include <cstdint>
#include <chrono>
#include <climits>

template <class LocationState, class AgentState>
class AgentTemplate
{
public:
  AgentTemplate(int id_, Location &loc_) 
  : loc(&loc_)
  , state(AgentState(id_))
  , rgen(std::chrono::system_clock::now().time_since_epoch().count())
  { }

  AgentTransition generate_transition(LocalMapping &local_mapping)
  {
    Direction new_dir = static_cast<Direction>((rgen() % 4) + 1);
    return AgentTransition(loc->state, state, new_dir);
  }

  Location *loc;
  AgentState state;
  parlay::random_generator rgen;
};

using Agent = AgentTemplate<DensityLocationState, DensityAgentState>;