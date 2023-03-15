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

class Agent 
{
public:
  Agent(int id_, Location &loc_)
  : loc(&loc_)
  , rgen(std::chrono::system_clock::now().time_since_epoch().count())
  , active(true)
  { }

  virtual AgentTransition generate_transition(LocalMapping &local_mapping) = 0;
  virtual AgentMessage generate_message(LocalMapping &local_mapping) = 0;

  Location *loc;
  AgentState *state;
  parlay::random_generator rgen;
  bool active;
};

template<class AgentState>
class WalkerAgentTemplate : public Agent
{
public:
  WalkerAgentTemplate(int id_, Location &loc_) 
  : Agent(id_, loc_)
  { 
    state = new AgentState(id_);
  }

  AgentTransition generate_transition(LocalMapping &local_mapping)
  {
    AgentState *new_astate = state;
    Direction new_dir = static_cast<Direction>((rgen() % 4) + 1);
    return AgentTransition(loc->state, std::move(new_astate), new_dir);
  }
};

template<class AgentState>
class CounterAgentTemplate : public Agent
{
public:
  CounterAgentTemplate(int id_, Location &loc_) 
  : Agent(id_, loc_)
  { 
    state = new AgentState(id_);
  }

  AgentTransition generate_transition(LocalMapping &local_mapping)
  {
    AgentState new_astate = *state;
    Direction new_dir = static_cast<Direction>((rgen() % 4) + 1);
    return AgentTransition(loc->state, std::move(new_astate), new_dir);
  }
};

using WalkerAgent = WalkerAgentTemplate<WalkerAgentState>;
using CounterAgent = CounterAgentTemplate<CounterAgentState>;