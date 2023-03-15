#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include "../template_states.h"

struct MultiLocationState
{
  MultiLocationState(
    Position location_
  )
    : location(location_)
  { }
  Position location;
};

struct WalkerAgentState
{
  WalkerAgentState(int id_)
  : id(id_)
  { }

  int id;
};

struct CounterAgentState
{
  CounterAgentState(int id_)
  : count(0)
  , rounds(0)
  , rnum(0)
  , id(id_)
  { }

  int count = 0;
  int rounds = 0;
  int rnum = 0;
  int id;
};

struct MultiAgentMessage
{

};

using Location                = LocationTemplate<MultiLocationState>;
using AgentTransition         = AgentTransitionTemplate<MultiLocationState, AgentState>;
using AgentMessage            = MultiAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage>*>>;
