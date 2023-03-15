#include <geo_utils.h>
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <set>

template<class Map>
void Configuration<Map>::update_agent(int i)
{
  auto &transition = agent_transitions[i];
  if (transition.accepted)
  {
    agents[agent_ids[i]].state = transition.astate;
    Position pos = get_coords_from_movement(agents[agent_ids[i]].loc->loc, transition.dir);
    agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y, pos.z);
  }
}

template<class Map>
Configuration<Map>::~Configuration() 
{ }

template<class Map>
void Configuration<Map>::custom_setup()
{ }

template<class Map>
void Configuration<Map>::update_location(Location *loc, parlay::slice<int *, int *> agentid_slice, parlay::slice<AgentTransition *, AgentTransition *> transitions)
{
  loopOverAgents(agentid_slice, [&](int i)
  {
    transitions[i].accepted = true;
    for(int j = 0; j < agentid_slice.size(); j++)
    {
      if (j != i && agents[agentid_slice[i]].rgen() % 100 < agents[agentid_slice[i]].percentage)
        transitions[i].astate.count++;
    }
    transitions[i].astate.rounds += 1;
  });
}

template<class Map>
bool Configuration<Map>::is_finished()
{
  return rounds >= HEIGHT / 2;
}

template<class Map>
void Configuration<Map>::print_config(int time, int flags)
{
  double density_estim = 0;
  int i = 0;
  for (auto agent : agents)
  {
    double estim = (double)agent.state.count / (double)agent.state.rounds;
    density_estim += estim;
    std::cout<<"Agent "<<i<<": "<<estim<<std::endl;
    i++;
  }
  std::cout << "Mean Density Estimation: " << density_estim / agents.size() << std::endl;
}