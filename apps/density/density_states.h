#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include "../../src/states.h"

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
  { }

  int id;
  int count = 0;
  int rounds = 0;
};

using Location = LocationTemplate<DensityLocationState, DensityAgentState>;
using ProposedAgentTransition = ProposedAgentTransitionTemplate <DensityAgentState>;
using LocalTransitory = LocalTransitoryTemplate<DensityLocationState, DensityAgentState>;
using AgentTransition = AgentTransitionTemplate<DensityLocationState, DensityAgentState>;
typedef std::map<Position, Location *> LocalMapping;
