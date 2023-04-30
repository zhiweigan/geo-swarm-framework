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
void Configuration<Map>::update_location(LocalMapping &loc, parlay::slice<int *, int *> agentid_slice, parlay::slice<AgentTransition *, AgentTransition *> transitions)
{
  Position delta = get_delta_from_movement(transitions[0].dir);
  if (transitions[0].dir == Direction::S)  {
    transitions[0].accepted = true;
    return;
  } 

  int a = 0;
  if (loc[delta].first->state.population->compare_exchange_weak(
    a, 1,
    std::memory_order_release, 
    std::memory_order_relaxed
  )) {
    *(loc[{0,0}].first->state.population) = 0;
    transitions[0].accepted = true;
  }
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
    if (unique_vertices[i]->state.heat > 0.05)
    {
      on_heat += unique_vertices[i]->state.count;
    } else
    {
      not_on_heat += unique_vertices[i]->state.count;
    }
  }

  if (flags & 0x1) {
    int count = 0;
    for(int i = 0; i < HEIGHT; i++)
    {
      for(int j = 0; j < WIDTH; j++)
      {
        if (map.get_vertex(i,j)->state.heat) count++;
        if (map.get_vertex(i,j)->state.heat < 0.05) std::cout<<blue;
        else if (map.get_vertex(i,j)->state.heat >= 0.05 && map.get_vertex(i,j)->state.heat < 0.4)  std::cout<<yellow;
        else std::cout<<red;
        std::cout<<std::min(map.get_vertex(i,j)->state.count, 9)<<def;
      }
      std::cout<<std::endl;
    }

    for(int i = 0; i < HEIGHT; i++)
    {
      for(int j = 0; j < WIDTH; j++)
      {
        if (map.get_vertex(i,j)->state.heat) count++;
        if (map.get_vertex(i,j)->state.heat < 0.05) std::cout<<"b";
        else if (map.get_vertex(i,j)->state.heat >= 0.05 && map.get_vertex(i,j)->state.heat < 0.4)  std::cout<<"y";
        else std::cout<<"r";
        // std::cout<<std::min(map.get_vertex(i,j)->state.count, 9)<<def;
      }
      std::cout<<std::endl;
    }

    // std::cout<<"count: "<<count<<std::endl;
  }
  
  
  std::cout<<"Iteration: "<<time<<" Percent, Number on heat: "<<on_heat / (on_heat + not_on_heat)<<", "<<on_heat<<std::endl;
}