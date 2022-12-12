#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include "../../src/states.h"

struct SampleLocationState
{
  SampleLocationState(
    Position location_
  )
    : location(location_)
  { }
  Position location;
};

struct SampleAgentState
{
  SampleAgentState(int id_) 
  : id(id_)
  , count(0)
  , rounds(0)
  { }

  int id;
  int count = 0;
  int rounds = 0;
};

struct SampleAgentMessage
{

};

#define MESSAGE
using Location                = LocationTemplate<SampleLocationState, SampleAgentState>;
using ProposedAgentTransition = ProposedAgentTransitionTemplate <SampleAgentState>;
using LocalTransitory         = LocalTransitoryTemplate<SampleLocationState, SampleAgentState>;
using AgentTransition         = AgentTransitionTemplate<SampleLocationState, SampleAgentState>;
using AgentMessage            = SampleAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage> *>>;
