#include "../../src/configuration.h"
#include "../../src/geo_utils.h"
#include "../../src/parlay/parallel.h"
#include "../../src/parlay/primitives.h"
#include <set>

void Configuration::update_agents()
{
  parlay::parallel_for(0, agent_ids.size(), [&](int i)
  {
    auto &transition = agent_transitions[i];
    transition.astate.count += counts[is_diff[i]+1] - counts[is_diff[i]] - 1; // TODO: abstract out more parallelism
    transition.astate.rounds += 1;
    agents[agent_ids[i]].state = transition.astate;
    Position pos = get_coords_from_movement(agents[agent_ids[i]].loc->loc, transition.dir);
    agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y); 
    transition.accepted = true;
  });
}

void Configuration::custom_setup()
{ }

void Configuration::update_locations()
{ }

bool Configuration::is_finished()
{
  return rounds >= n / 2;
}

void Configuration::print_config(int time, int flags)
{
  std::cout << "Agent Locations: " << std::endl;
  double density_estim = 0;
  for (auto agent : agents)
  {
    double estim = (double)agent.state.count / (double)agent.state.rounds;
    std::cout<<estim<<", ";
    density_estim += estim;
  }
  std::cout<<std::endl;
  std::cout << density_estim / agents.size() << std::endl;
}