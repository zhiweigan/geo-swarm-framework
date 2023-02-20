#include <configuration.h>
#include <geo_utils.h>
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <set>

// must be defined here so the type is complete
Configuration::~Configuration() { }

void Configuration::custom_setup()
{ }

void Configuration::update_agent(int i)
{
  auto &transition = agent_transitions[i];
  if (agent_transitions[i].accepted)
  {
    agents[agent_ids[i]].state = transition.astate;
    Position pos = get_coords_from_movement(agents[agent_ids[i]].loc->loc, transition.dir);
    agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y);
  }
  if (agents[agent_ids[i]].state.committed_task != nullptr)
  {
    agents[agent_ids[i]].active = false;
  }
}

void Configuration::update_location(Location *loc, parlay::slice<int *, int *> agents)
{
  loopOverAgents(agents, [&](int i) {
    if (i < loc->state.residual_demand)
    {
      agent_transitions[i].accepted = true;
    }
  });
}

void Configuration::set_task_vertex(Position & pos)
{
  task_vertices.emplace_back(pos.x, pos.y);
}

bool Configuration::is_finished()
{
  int total_rd = parlay::reduce(parlay::delayed_map(*get_tasks(), [&](Position task_vertex)
  { 
      return get_vertex(task_vertex.x, task_vertex.y)->state.residual_demand; 
  }));
  return total_rd == 0;
}

Location *Configuration::get_task(int i)
{
  return map.get_vertex(task_vertices[i].x, task_vertices[i].y);
}

void Configuration::print_config(int time, int flags)
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