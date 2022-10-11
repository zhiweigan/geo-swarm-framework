#pragma once
#include "states.h"
#include "parlay/random.h"
#include <algorithm>

struct ProposedAgentTransition
{
  AgentState astate;
  Direction dir;
};

struct LocalTransitory
{
  LocationState loc_state;
  std::map<int16_t, ProposedAgentTransition> agent_updates;
};

inline LocalTransitory naive_resolution(
  std::map<int16_t, LocationState> proposed_vertex_updates,
  std::map<int16_t, ProposedAgentTransition> proposed_agent_updates,
  Location *vertex)
{
  return {proposed_vertex_updates.begin()->second, proposed_agent_updates};
}

inline LocalTransitory task_claiming_resolution(
  std::map<int16_t, LocationState> proposed_vertex_updates, 
  std::map<int16_t, ProposedAgentTransition> proposed_agent_updates, 
  Location* vertex)
{
  if (!vertex->state.is_task)
  {
    return naive_resolution(proposed_vertex_updates, proposed_agent_updates, vertex);
  }

  int attempted_claims = 0;
  std::vector<uint16_t> contenders;
  int available_slots = vertex->state.residual_demand;
  for (auto const &[agent_id, update] : proposed_agent_updates)
  {
    if (proposed_vertex_updates.at(agent_id).residual_demand == vertex->state.residual_demand - 1)
    {
      attempted_claims += (vertex->state.residual_demand - proposed_vertex_updates.at(agent_id).residual_demand);
      contenders.push_back(agent_id);
    }
  }

  if (attempted_claims <= available_slots)
  {
    vertex->state.residual_demand = available_slots - attempted_claims;
    return {vertex->state, proposed_agent_updates};
  }
  else
  {
    std::map<int16_t, ProposedAgentTransition> new_proposed_agent_updates;
    parlay::sequence<uint16_t> winners = parlay::random_shuffle(contenders);
    
    for (int i = 0; i < available_slots; i++)
    {
      new_proposed_agent_updates.insert_or_assign(winners[i], proposed_agent_updates[winners[i]]);
    }
    for (int i = available_slots; i < winners.size(); i++)
    {
      proposed_agent_updates.at(winners[i]).astate.committed_task = nullptr;
      new_proposed_agent_updates.insert_or_assign(winners[i], proposed_agent_updates[winners[i]]);
    }
    vertex->state.residual_demand = 0;
    return { vertex->state, new_proposed_agent_updates};
  }
}