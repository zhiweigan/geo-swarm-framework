#include "agent.h"
#include "states.h"
#include "geo_utils.h"
#include <map>
#include <climits>

LocationState *Agent::find_nearby_task(LocalMapping &local_mapping)
{
  LocationState *return_state = nullptr;
  double min_dist = __DBL_MAX__;
  for (auto const & [pos, vtx] : local_mapping)
  {
    if (vtx->state.is_task && vtx->state.residual_demand > 0)
    {
      double dist = l2_distance(vtx->loc.x, vtx->loc.y, vtx->loc.x + pos.x, vtx->loc.y + pos.y);
      if (dist < min_dist)
      {
        min_dist = dist;
        return_state = &vtx->state;
      }
    }
  }
  return return_state;
}

bool Agent::within_site(int x, int y)
{
  return x >= 0 && x < HEIGHT && y >= 0 && y < WIDTH;
}

// double levy(double x)
// {
//   return (1.0 / sqrt(2.0 * M_PI * pow(x, 2.0)) * exp(-1.0 / (2.0 * x)));
// }

Direction Agent::get_travel_direction(AgentState &new_agent_state)
{
  if (state.travel_distance <= 0)
  {
    new_agent_state.travel_distance = abs(loc->loc.x - state.starting_point.x) + abs(loc->loc.y - state.starting_point.y);
    new_agent_state.angle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0 * M_PI)));
    new_agent_state.starting_point = Position{loc->loc.x, loc->loc.y};
  }

  Direction new_direction = get_direction_from_angle(new_agent_state.angle, new_agent_state.starting_point, Position{loc->loc.x, loc->loc.y});
  Position new_location = get_coords_from_movement(Position{loc->loc.x, loc->loc.y}, new_direction, true);

  while (!within_site(new_location.x, new_location.y))
  {
    new_agent_state.angle = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (2.0 * M_PI)));
    new_agent_state.starting_point = Position{loc->loc.x, loc->loc.y};
    new_direction = get_direction_from_angle(new_agent_state.angle, new_agent_state.starting_point, Position{loc->loc.x, loc->loc.y});
    new_location = get_coords_from_movement(Position{loc->loc.x, loc->loc.y}, new_direction, true);
  }
  new_agent_state.travel_distance -= 1;
  return new_direction;
}

AgentTransition Agent::generate_transition(LocalMapping &local_mapping)
{
  AgentState new_astate = state;
  if (!state.committed_task && !state.destination_task)
  {
    LocationState* nearby_task = find_nearby_task(local_mapping);
    if (nearby_task)
    {
      new_astate.destination_task = nearby_task;
      return {loc->state, new_astate, Direction::S};
    }
    else
    {
      Direction new_dir = get_travel_direction(new_astate);
      return {loc->state, new_astate, new_dir};
    }
  }
  if (state.destination_task)
  {
    if (state.destination_task->residual_demand == 0)
    {
      new_astate.destination_task = nullptr;
      return {loc->state, new_astate, Direction::S}; 
    }
    if (loc->loc == state.destination_task->task_location)
    {
      new_astate.committed_task = state.destination_task;
      new_astate.destination_task = nullptr;
      LocationState new_vstate = loc->state;
      new_vstate.residual_demand -= 1;
      return {new_vstate, new_astate, Direction::S};
    }
    else
    {
      Direction new_direction = get_direction_from_destination(state.destination_task->task_location, loc->loc);
      Position new_location = get_coords_from_movement(loc->loc, new_direction);
      return {loc->state, new_astate, new_direction};
    }
  }
  else
  {
    return {loc->state, state, Direction::S};
  }
}