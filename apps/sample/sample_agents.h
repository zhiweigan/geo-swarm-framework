#pragma once
#include "sample_states.h"
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

  AgentMessage generate_message(LocalMapping &local_mapping)
  {
    return AgentMessage({state.color});
  }

  AgentTransition generate_transition(LocalMapping &local_mapping)
  {
    int num_red = 0;
    for (uint i = 0; i < local_mapping[{0, 0}].second->size(); i++)
    {
      SampleColor col = local_mapping[{0, 0}].second->at(i).color;
      if (col == SampleColor::RED) num_red += 1;
      else if (col == SampleColor::BLUE) num_red -= 1;
    }
    AgentState new_astate = state;
    if (new_astate.color == SampleColor::NONE) {
      if (num_red > 0)
        new_astate.color = SampleColor::RED;
      else if (num_red == 0)
        new_astate.color = SampleColor::NONE;
      else
        new_astate.color = SampleColor::BLUE;
    }
    Direction new_dir = static_cast<Direction>((rgen() % 4) + 1);
    return AgentTransition(loc->state, std::move(new_astate), new_dir);
  }

  Location *loc;
  AgentState state;
  parlay::random_generator rgen;
};

using Agent = AgentTemplate<SampleLocationState, SampleAgentState>;