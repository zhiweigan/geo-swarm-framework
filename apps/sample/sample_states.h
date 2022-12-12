#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include "../../src/states.h"

enum SampleColor
{
  NONE,
  RED,
  BLUE,
};

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
  , color(SampleColor::NONE)
  { }

  int id;
  SampleColor color;
};

struct SampleAgentMessage
{
  SampleColor color;
};

#define MESSAGE
#define PRINT_OUT 0x000001
using Location                = LocationTemplate<SampleLocationState, SampleAgentState>;
using ProposedAgentTransition = ProposedAgentTransitionTemplate <SampleAgentState>;
using LocalTransitory         = LocalTransitoryTemplate<SampleLocationState, SampleAgentState>;
using AgentTransition         = AgentTransitionTemplate<SampleLocationState, SampleAgentState>;
using AgentMessage            = SampleAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage> *>>;
