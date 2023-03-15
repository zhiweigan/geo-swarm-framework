#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include "../template_states_3d.h"

struct DensityLocationState
{
  DensityLocationState(
    Position location_
  )
    : location(location_)
  { }
  Position location;
};

struct DensityAgentState
{
  DensityAgentState(int id_) 
  : id(id_)
  , count(0)
  , rounds(0)
  , rnum(0)
  { }

  int id;
  int count = 0;
  int rounds = 0;
  int rnum = 0;
};

struct DensityAgentMessage
{

};

using Location                = LocationTemplate<DensityLocationState>;
using AgentTransition         = AgentTransitionTemplate<DensityLocationState, DensityAgentState>;
using AgentMessage            = DensityAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage>*>>;
