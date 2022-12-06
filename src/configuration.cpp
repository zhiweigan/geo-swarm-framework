#include "configuration.h"
#include "geo_utils.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include <set>

void Configuration::init()
{
  for (int16_t x = 0; x < HEIGHT; x++)
  {
    for (int16_t y = 0; y < WIDTH; y++)
    {
      LocalMapping local_mapping = generate_local_mapping(*map.get_vertex(x, y), influence_radius, map);
      local_mappings.insert_or_assign(Position{x, y}, local_mapping);
    }
  }
}

// add new function to set up parallelism, based on HEIGHT, WIDTH, NUMAGENTS
void Configuration::add_agents(std::vector<Position> &agent_pos) 
{
  int id = 0;
  for (Position pos : agent_pos)
  {
    map.get_vertex(pos.x, pos.y)->agents_seen.insert(id);
    Agent agent(id++, *map.get_vertex(pos.x, pos.y));
    agents.push_back(agent);
    agent_transitions.push_back(AgentTransition({pos}, {id-1}));
  }
}

void
Configuration::transition()
{
  // sort agents by location TODO: semisort
  auto sort_start = std::chrono::high_resolution_clock::now();
  auto comp = [&](int agent1, int agent2)
  {
    return agents[agent1].loc->loc < agents[agent2].loc->loc;
  };
  parlay::sort_inplace(agent_ids, comp);
  auto sort_end = std::chrono::high_resolution_clock::now();
  sorting += std::chrono::duration_cast<std::chrono::nanoseconds>(sort_end - sort_start).count();
  
  // generate transitions for each agent
  auto transition_start = std::chrono::high_resolution_clock::now();
  parlay::parallel_for(0, agent_ids.size() - 1, [&](int i)
  { 
    agent_transitions[i] = agents[agent_ids[i]].generate_transition(local_mappings[agents[agent_ids[i]].loc->loc]); 
  });
  auto transition_end = std::chrono::high_resolution_clock::now();
  transitions += std::chrono::duration_cast<std::chrono::nanoseconds>(transition_end - transition_start).count();

  // get array differecnes
  auto update_start = std::chrono::high_resolution_clock::now();
  update_config();
  auto update_end = std::chrono::high_resolution_clock::now();
  update += std::chrono::duration_cast<std::chrono::nanoseconds>(update_end - update_start).count();
}

parlay::sequence<Position> *Configuration::get_tasks()
{
  return &task_vertices;
}

Location *Configuration::get_vertex(int x, int y)
{
  return map.get_vertex(x, y);
}