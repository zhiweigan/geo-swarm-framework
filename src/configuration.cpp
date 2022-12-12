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
  custom_setup();
}

void Configuration::parallel_setup()
{
  for (Agent agent : agents)
  {
    loc_diff.push_back(false);
    is_diff.push_back(0);
    offsets.push_back(0);
    unique_vertices.push_back(0);
    counts.push_back(0);
    agent_ids.push_back(agent.state.id);
    removed_agent_ids.push_back(-1);
  }
}

// add new function to set up parallelism, based on HEIGHT, WIDTH, NUMAGENTS
void Configuration::add_agents(std::vector<Position> &agent_pos) 
{
  int id = 0;
  for (Position pos : agent_pos)
  {
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

  auto parallel_start = std::chrono::high_resolution_clock::now();
  parlay::parallel_for(0, agents.size(), [&](size_t i)
  {
    loc_diff[i] = 0;
    is_diff[i] = 0;
    offsets[i] = 0; 
  });

  parlay::parallel_for(0, agent_ids.size() - 1, [&](size_t i)
  {
    if (agents[agent_ids[i]].loc->loc != agents[agent_ids[i+1]].loc->loc){
      loc_diff[i] = i+1;
      is_diff[i] = 1;
    } 
  });
  loc_diff[agent_ids.size() - 1] = agent_ids.size();
  is_diff[agent_ids.size() - 1] = 1;

  // get offsets of differences in sorted array
  auto offset_filter = [&](int x) { return x != 0; };
  size_t num_unique_locations = parlay::filter_into_uninitialized(loc_diff, offsets, offset_filter);

  // save the unique vertices into an array for future use
  parlay::parallel_for(0, num_unique_locations, [&](size_t i)
  { 
    unique_vertices[i] = agents[agent_ids[offsets[i] - 1]].loc; 
  });

  counts[0] = offsets[0];
  parlay::parallel_for(1, num_unique_locations, [&](size_t i)
  { 
    counts[i] = offsets[i] - offsets[i - 1]; 
  });
  // update messages for each location
  parlay::scan_inplace(counts);
  parlay::scan_inplace(is_diff);
  auto parallel_end = std::chrono::high_resolution_clock::now();
  parallel += std::chrono::duration_cast<std::chrono::nanoseconds>(parallel_end - parallel_start).count();

#ifdef MESSAGE
  auto messages_start = std::chrono::high_resolution_clock::now();
  parlay::parallel_for(0, agent_ids.size() - 1, [&](int i)
  {
    agent_messages[i] = agents[agent_ids[i]].generate_message(local_mappings[agents[agent_ids[i]].loc->loc]); 
  });

  parlay::parallel_for(0, num_unique_locations, [&](int i)
  {
    Position loc = unique_vertices[i]->loc;
    for(int j = counts[i]; j < counts[i+1]; j++)
    {
      map.add_message(loc.x, loc.y, std::move(agent_messages[j]));
    }
  });
  auto messages_end = std::chrono::high_resolution_clock::now();
  messages += std::chrono::duration_cast<std::chrono::nanoseconds>(messages_end - messages_start).count();
#endif

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
  update_agents();
  update_locations();
  auto update_end = std::chrono::high_resolution_clock::now();
  update += std::chrono::duration_cast<std::chrono::nanoseconds>(update_end - update_start).count();

  // remove messages generated this round
  parlay::parallel_for(0, num_unique_locations, [&](int i)
  {
    Position loc = unique_vertices[i]->loc;
    map.get_messages(loc.x, loc.y)->resize(0);
  });
}

parlay::sequence<Position> *Configuration::get_tasks()
{
  return &task_vertices;
}

Location *Configuration::get_vertex(int x, int y)
{
  return map.get_vertex(x, y);
}