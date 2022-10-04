#include "configuration.h"
#include <set>

void Configuration::add_agents(parlay::sequence<Position> &agent_pos) 
{
  int16_t id = 0;
  for (Position pos : agent_pos)
  {
    map.get_vertex(pos.x, pos.y)->agents_seen.insert(id);
    Agent agent(id++, *map.get_vertex(pos.x, pos.y));
    agents.push_back(agent);
  }
}

void Configuration::reset_agents(parlay::sequence<Position> &agent_pos)
{
  for (int i = 0; i < n; i++) {
    for(int j = 0; j < m; j++) {
      map.get_vertex(i, j)->agents_seen = std::set<int16_t>();
    }
  }
  add_agents(agent_pos);
}

bool Configuration::execute_transition()
{
  return false;
}

void Configuration::transition()
{

}

void Configuration::set_task_vertex(Position &pos)
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

Location* Configuration::get_vertex(int x, int y)
{
  return map.get_vertex(x, y);
}

Location* Configuration::get_task(int i)
{
  return map.get_vertex(task_vertices[i].x, task_vertices[i].y);
}