#include "../../src/configuration.h"
#include "../../src/geo_utils.h"
#include "../../src/parlay/parallel.h"
#include "../../src/parlay/primitives.h"
#include <set>

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

void Configuration::update_config()
{
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

  

  // handle agents that need to move
  // parlay::scan_inplace(counts);
  parlay::scan_inplace(is_diff);

  parlay::parallel_for(0, agent_ids.size(), [&](int i)
  {
    auto &transition = agent_transitions[i];
    transition.astate.count += counts[is_diff[i]] - 1;
    transition.astate.rounds += 1;
    agents[agent_ids[i]].state = transition.astate;
    Position pos = get_coords_from_movement(agents[agent_ids[i]].loc->loc, transition.dir);
    agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y);
  });
}

bool Configuration::is_finished()
{
  return false;
}

void Configuration::print_config(int time)
{  
  std::cout << "Agent Locations: " << std::endl;
  double density_estim = 0;
  for (auto agent : agents)
  {
    double estim = (double)agent.state.count / (double)agent.state.rounds;
    density_estim += estim;
    // std::cout << agent.loc->loc << " " << estim << std::endl;
  }
  std::cout<<density_estim / agents.size() <<std::endl;
}