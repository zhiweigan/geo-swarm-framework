#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <atomic>
#include <constants.h>
#include <set>
#include <map>
#include "../template_states.h"

struct AggregateLocationState
{
  AggregateLocationState(
    Position location_
  )
    : location(location_)
    , heat(0)
    , count(0)
    , isWall(false)
    , population(0)
  { }
  Position location;
  double heat;
  bool isWall;
  int count;
  std::atomic<int>* population;
};

struct AggregateAgentState
{
  AggregateAgentState(int id_) 
  : id(id_)
  , responsiveness(RESPONSIVE)
  , count(0)
  , wait_left(0)
  , angle(-1)
  , settled(false)
  { }

  int id;
  double responsiveness;
  int wait_left;
  int count;
  Position starting_point;
  float angle;
  bool settled;
};

struct AggregateAgentMessage
{
  int present = 0;
  double heat = 0;
};

#define MESSAGE
using Location                = LocationTemplate<AggregateLocationState>;
using AgentTransition         = AgentTransitionTemplate<AggregateLocationState, AggregateAgentState>;
using AgentMessage            = AggregateAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage>*>>;
