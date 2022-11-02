#include "configuration.h"
#include "geo_utils.h"
#include "parlay/parallel.h"
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
  int16_t id = 0;
  for (Position pos : agent_pos)
  {
    map.get_vertex(pos.x, pos.y)->agents_seen.insert(id);
    Agent agent(id++, *map.get_vertex(pos.x, pos.y));
    agents.push_back(agent);
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

void Configuration::execute_transition()
{
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < m; j++)
    {
      LocalTransitory* delta = map.get_transition(i, j);
      Location *vtx = map.get_vertex(i, j);
      vtx->state = delta->loc_state;

      for (auto const &[agent_id, update] : delta->agent_updates)
      {
        Agent *agent = &agents[agent_id];
        agent->state = update.astate;
        vtx->agents_seen.erase(agent_id);
        Position pos = get_coords_from_movement(vtx->loc, update.dir);
        agent->loc = map.get_vertex(pos.x, pos.y);
        agent->loc->agents_seen.insert(agent_id);
      }
    }
  }
}

void Configuration::delta(std::map<Position, Location *> &local_mapping)
{
  Location *vtx = local_mapping[{0, 0}];
  if (vtx->agents_seen.size() == 0)
  {
    LocalTransitory transition{vtx->state, {}};
    map.set_transition(vtx->loc.x, vtx->loc.y, std::move(transition));
    return;
  }

  std::map<int16_t, LocationState> proposed_vertex_states;
  std::map<int16_t, ProposedAgentTransition> proposed_agent_states;

  for (int agent_id : vtx->agents_seen)
  {
    AgentTransition transition = agents[agent_id].generate_transition(local_mapping);
    proposed_vertex_states.insert_or_assign(agent_id, transition.lstate);
    proposed_agent_states.insert_or_assign(agent_id, ProposedAgentTransition{transition.astate, transition.dir});
  }

  map.set_transition(vtx->loc.x, vtx->loc.y, task_claiming_resolution(proposed_vertex_states, proposed_agent_states, vtx));
}

void Configuration::generate_global_transitory()
{
  parlay::parallel_for(0, HEIGHT, [&](int16_t x)
  {
    for (int16_t y = 0; y < WIDTH; y++)
    {
      delta(local_mappings[Position{x, y}]);
    } 
  });
}

void
Configuration::transition()
{
  generate_global_transitory();
  execute_transition();
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