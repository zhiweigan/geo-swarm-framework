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
  assert(agent_pos.size() < MAX_AGENTS);
  int16_t id = 0;
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
  }
}

void Configuration::reset_agents(std::vector<Position> &agent_pos)
{
  for (int i = 0; i < n; i++) {
    for(int j = 0; j < m; j++) {
      map.get_vertex(i, j)->agents_seen = std::set<int16_t>();
    }
  }
  add_agents(agent_pos);
}

void
Configuration::transition()
{
  // sort agents by location TODO: semisort
  auto comp = [&](Agent agent1, Agent agent2)
  {
    return agent1.loc->loc < agent2.loc->loc;
  };
  parlay::sort_inplace(agents, comp);
  // parlay::integer_sort_inplace(agents, [&] (Agent agent){
  //   uint x = agent.loc->loc.x;
  //   uint y = agent.loc->loc.y;
  //   return ((x + y) * (x + y + 1) / 2) + y;
  // });

  // generate transitions for each agent
  parlay::parallel_for(0, agents.size()-1, [&](int i)
  { 
    agent_transitions[i] = agents[i].generate_transition(local_mappings[agents[i].loc->loc]); 
  });

  // get array differecnes
  parlay::parallel_for(0, agents.size()-1, [&](size_t i)
  {
    loc_diff[i] = 0;
    is_diff[i] = 0;
  });

  parlay::parallel_for(0, agents.size()-1, [&](size_t i)
  {
    if (agents[i].loc->loc != agents[i+1].loc->loc){
      loc_diff[i] = i+1;
      is_diff[i] = 1;
    } 
  });
  loc_diff[agents.size() - 1] = agents.size();
  is_diff[agents.size() - 1] = 1;

  // get offsets of differences in sorted array
  auto offset_filter = [&](int x)
  { return x != 0; };
  size_t num_unique_locations = parlay::filter_into_uninitialized(loc_diff, offsets, offset_filter); // make this in-place

  // save the unique vertices into an array for future use
  parlay::parallel_for(0, num_unique_locations, [&](size_t i)
  { 
    unique_vertices[i] = agents[offsets[i] - 1].loc; 
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
      unique_vertices[i]->state.residual_demand -= std::min((int16_t)counts[i], unique_vertices[i]->state.residual_demand);
    } 
  });

  // handle agents that need to move
  parlay::scan_inplace(counts);
  parlay::scan_inplace(is_diff);
  parlay::parallel_for(0, agents.size(), [&](int i)
  {
    if (!agents[i].loc->state.is_task ||
        (agents[i].loc->state.is_task && i <= counts[is_diff[i]] + agents[i].loc->state.residual_demand))
    {
      auto &transition = agent_transitions[i];
      agents[i].state = transition.astate;
      Position pos = get_coords_from_movement(agents[i].loc->loc, transition.dir);
      agents[i].loc = map.get_vertex(pos.x, pos.y);
    }
  });
}

void Configuration::set_task_vertex(Position & pos)
{
  task_vertices.emplace_back(pos.x, pos.y);
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