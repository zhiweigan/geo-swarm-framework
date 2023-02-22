#include <configuration.h>
#include <geo_utils.h>
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <set>

void Configuration::update_agent(int i)
{
  auto &transition = agent_transitions[i];
  if (transition.accepted)
  {
    agents[agent_ids[i]].state = transition.astate;
    Position pos = get_coords_from_movement(agents[agent_ids[i]].loc->loc, transition.dir);
    agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y);
  }
}

Configuration::~Configuration() 
{ }

void Configuration::custom_setup()
{ }

void Configuration::update_location(Location *loc, parlay::slice<int *, int *> agentid_slice, parlay::slice<AgentTransition *, AgentTransition *> transitions)
{
  loopOverAgents(agentid_slice, [&](int i)
  {
    transitions[i].accepted = true;
    transitions[i].astate.count += agentid_slice.size() - 1;
    transitions[i].astate.rounds += 1;
  });
}

bool Configuration::is_finished()
{
  return rounds >= n / 2;
}

void Configuration::print_config(int time, int flags)
{
  double density_estim = 0;
  for (auto agent : agents)
  {
    double estim = (double)agent.state.count / (double)agent.state.rounds;
    density_estim += estim;
  }
  std::cout << "Density Estimation: " << density_estim / agents.size() << std::endl;
}