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

void Configuration::add_agents(std::vector<Position> &agent_pos) 
{
  int id = 0;
  for (Position pos : agent_pos)
  {
    map.get_vertex(pos.x, pos.y)->agents_seen.insert(id);
    Agent agent(id++, *map.get_vertex(pos.x, pos.y));
    agents.push_back(agent);
    agent_transitions.push_back(AgentTransition{LocationState{pos}, AgentState{}, Direction::S});

    // parallel stuff
    loc_diff.push_back(false);
    is_diff.push_back(0);
    offsets.push_back(0);
    unique_vertices.push_back(0);
    counts.push_back(0);
    agent_ids.push_back(id-1);
  }
}

void Configuration::reset_agents(std::vector<Position> &agent_pos)
{
  for (int i = 0; i < n; i++) {
    for(int j = 0; j < m; j++) {
      map.get_vertex(i, j)->agents_seen = std::set<int>();
    }
  }
  add_agents(agent_pos);
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
  // parlay::integer_sort_inplace(agents, [&](Agent agent)
  // {
  //   uint x = agent.loc->loc.x;
  //   uint y = agent.loc->loc.y;
  //   return ((x + y) * (x + y + 1) / 2) + y; 
  // });
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
  auto offset_filter = [&](int x)
  { return x != 0; };
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

  // update location states
  parlay::parallel_for(0, num_unique_locations, [&](int i)
  {
    if (unique_vertices[i]->state.is_task)
    {
      unique_vertices[i]->state.residual_demand -= std::min(counts[i], unique_vertices[i]->state.residual_demand);
    } 
  });

  // handle agents that need to move
  parlay::scan_inplace(counts);
  parlay::scan_inplace(is_diff);
  parlay::parallel_for(0, agent_ids.size(), [&](int i)
  {
    if (!agents[agent_ids[i]].loc->state.is_task ||
        (agents[agent_ids[i]].loc->state.is_task && i <= counts[is_diff[i]] + agents[agent_ids[i]].loc->state.residual_demand))
    {
      auto &transition = agent_transitions[i];
      agents[agent_ids[i]].state = transition.astate;
      Position pos = get_coords_from_movement(agents[agent_ids[i]].loc->loc, transition.dir);
      agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y);
    } 
  });
  agent_ids = parlay::filter(agent_ids, [&](int agent)
  {
    return agents[agent].state.committed_task == nullptr;
  });
  auto update_end = std::chrono::high_resolution_clock::now();
  update += std::chrono::duration_cast<std::chrono::nanoseconds>(update_end - update_start).count();
}

void Configuration::set_task_vertex(Position & pos)
{
  task_vertices.emplace_back(pos.x, pos.y);
}

parlay::sequence<Position> *Configuration::get_tasks()
{
  return &task_vertices;
}

bool Configuration::all_agents_terminated()
{
  return false;
}

bool Configuration::all_tasks_completed()
{
  return false;
}

Location *Configuration::get_vertex(int x, int y)
{
  return map.get_vertex(x, y);
}

Location *Configuration::get_task(int i)
{
  return map.get_vertex(task_vertices[i].x, task_vertices[i].y);
}

void Configuration::print_config(int time)
{
  std::cout << "map @ time" << time << std::endl;
  std::vector<std::string> output;
  for (int i = 0; i < n; i++)
  {
    std::string s = "";
    for (int j = 0; j < m; j++)
    {
      s += '0';
    }
    output.push_back(s);
  }
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < m; j++)
    {
      Location *loc = map.get_vertex(i, j);
      if (loc->state.is_home)
        output[i][j] = 'h';
      if (loc->state.is_task)
        output[i][j] = std::to_string(loc->state.residual_demand)[0];
    }
  }
  for (int i = 0; i < n; i++)
  {
    std::cout << output[i] << std::endl;
  }

  std::cout << "Agent Locations: " << std::endl;
  for (auto agent : agents)
  {
    std::cout << "(" << agent.loc->loc.x << ", " << agent.loc->loc.y << ")" << std::endl;
  }
}