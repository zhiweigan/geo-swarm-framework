#include <geo_utils.h>
#include <parlay/parallel.h>
#include "colorprint.h"
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
    agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y);
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
      if (j != i)
        transitions[i].astate.count++;
    }
  });
}

template<class Map>
bool Configuration<Map>::is_finished()
{
  return rounds >= MAX_ITERS;
}

template<class Map>
void Configuration<Map>::print_config(int time, int flags)
{
  Color::Modifier red(Color::FG_RED);
  Color::Modifier blue(Color::FG_BLUE);
  Color::Modifier yellow(Color::FG_YELLOW);
  Color::Modifier def(Color::FG_DEFAULT);

  for(int i = 0; i < HEIGHT; i++)
  {
    for(int j = 0; j < WIDTH; j++)
    {
      map.get_vertex(i,j)->state.count = 0;
    }
  }

  double on_heat = 0;
  double not_on_heat = 0;

  for(int i = 0; i < num_unique_locations; i++)
  {
    unique_vertices[i]->state.count = getAgentsAtUniqueLocation(i).size();
    if (unique_vertices[i]->state.heat > 0)
    {
      on_heat += unique_vertices[i]->state.count;
    } else
    {
      not_on_heat += unique_vertices[i]->state.count;
    }
  }

  for(int i = 0; i < HEIGHT; i++)
  {
    for(int j = 0; j < WIDTH; j++)
    {
      if (map.get_vertex(i,j)->state.heat == 0) std::cout<<blue;
      else if (map.get_vertex(i,j)->state.heat > 0 && map.get_vertex(i,j)->state.heat < 0.4)  std::cout<<yellow;
      else std::cout<<red;
      std::cout<<std::min(map.get_vertex(i,j)->state.count, 9)<<def;
    }
    std::cout<<std::endl;
  }

  std::cout<<"Percent on heat: "<<on_heat / (on_heat + not_on_heat)<<std::endl;
  std::cout<<"Number on heat: "<<on_heat<<std::endl;
}