#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <map>
#include "../template_states.h"

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

struct SampleAgentStateCopy
{
  SampleAgentStateCopy(int id_)
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
using Location                = LocationTemplate<SampleLocationState>;
using AgentTransition         = AgentTransitionTemplate<SampleLocationState, SampleAgentState>;
using AgentTransitionCopy     = AgentTransitionTemplate<SampleLocationState, SampleAgentStateCopy>;
using AgentMessage            = SampleAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage> *>>;
