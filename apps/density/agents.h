#pragma once
#include <states.h>
#include <map.h>
#include <constants.h>
#include <parlay/random.h>
#include <geo_utils.h>
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
  , percentage(1.0)
  , active(true)
  { }

  AgentTransition generate_transition(LocalMapping &local_mapping)
  {
    AgentState new_astate = state;
    int rand = rgen() % 10;
    Direction new_dir;
    // only right and up
    if (rand < 0) new_dir = static_cast<Direction>(1);
    // else if (rand < 5) new_dir = static_cast<Direction>(2);
    // else if (rand < 5) new_dir = static_cast<Direction>(3);
    else new_dir = static_cast<Direction>(4); 
    new_astate.rnum = rgen();
    return AgentTransition(loc->state, std::move(new_astate), new_dir);
  }

  Location *loc;
  AgentState state;
  parlay::random_generator rgen;
  bool active;
  int percentage;
};

using Agent = AgentTemplate<DensityLocationState, DensityAgentState>;