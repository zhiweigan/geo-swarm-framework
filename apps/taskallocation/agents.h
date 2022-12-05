#pragma once
#include "states.h"
#include "map.h"
#include "constants.h"
#include "parlay/random.h"
#include "agent.h"
#include "geo_utils.h"
#include <map>
#include <cstdint>
#include <chrono>
#include <climits>

template <class LocationState, class AgentState>
class AgentTemplate
{
public:
  AgentTemplate(int id_, Location &loc_)
  : loc(&loc_)
  , state(AgentState(id_, loc_, 0.0))
  , rgen(std::chrono::system_clock::now().time_since_epoch().count())
  { }

  Location *find_nearby_task(LocalMapping &local_mapping)
  {
    Location *return_state = nullptr;
    Position origin = local_mapping[Position{0, 0}]->loc;
    double min_dist = __DBL_MAX__;
    for (auto &[pos, vtx] : local_mapping)
    {
      (void)pos;
      if (vtx->state.is_task && vtx->state.residual_demand > 0)
      {
        double dist = l2_distance(origin.x, origin.y, vtx->loc.x, vtx->loc.y);
        if (dist < min_dist)
        {
          min_dist = dist;
          return_state = vtx;
        }
      }
    }
    return return_state;
  }

  AgentTransition generate_transition(LocalMapping &local_mapping)
  {
    AgentState new_astate = state;
    if (!state.committed_task && !state.destination_task)
    {
      Location *nearby_task = find_nearby_task(local_mapping);
      if (nearby_task)
      {
        new_astate.destination_task = nearby_task;
        return AgentTransition(loc->state, std::move(new_astate), Direction::S);
      }
      else
      {
        Direction new_dir = get_travel_direction(new_astate);
        return AgentTransition(loc->state, std::move(new_astate), std::move(new_dir));
      }
    }
    if (state.destination_task)
    {
      if (state.destination_task->state.residual_demand == 0)
      {
        new_astate.destination_task = nullptr;
        return AgentTransition(loc->state, new_astate, Direction::S);
      }
      if (loc->loc == state.destination_task->state.task_location)
      {
        new_astate.committed_task = state.destination_task;
        new_astate.destination_task = nullptr;
        LocationState new_vstate = loc->state;
        new_vstate.residual_demand -= 1;
        return AgentTransition(new_vstate, std::move(new_astate), Direction::S);
      }
      else
      {
        Direction new_direction = get_direction_from_destination(state.destination_task->state.task_location, loc->loc);
        return {loc->state, std::move(new_astate), std::move(new_direction)};
      }
    }
    else
    {
      return {loc->state, state, Direction::S};
    }
  }

  Direction get_travel_direction(AgentState &new_agent_state)
  {
    if (state.travel_distance <= 0)
    {
      new_agent_state.travel_distance = abs(loc->loc.x - state.starting_point.x) + abs(loc->loc.y - state.starting_point.y);
      new_agent_state.angle = static_cast<float>(rgen()) / (static_cast<float>(RAND_MAX / (2.0 * M_PI)));
      new_agent_state.starting_point = Position{loc->loc.x, loc->loc.y};
    }

    Direction new_direction = get_direction_from_angle(new_agent_state.angle, new_agent_state.starting_point, Position{loc->loc.x, loc->loc.y});
    Position new_location = get_coords_from_movement(Position{loc->loc.x, loc->loc.y}, new_direction, true);

    while (!within_site(new_location.x, new_location.y))
    {
      new_agent_state.angle = static_cast<float>(rgen()) / (static_cast<float>(RAND_MAX / (2.0 * M_PI)));
      new_agent_state.starting_point = Position{loc->loc.x, loc->loc.y};
      new_direction = get_direction_from_angle(new_agent_state.angle, new_agent_state.starting_point, Position{loc->loc.x, loc->loc.y});
      new_location = get_coords_from_movement(Position{loc->loc.x, loc->loc.y}, new_direction, true);
    }
    new_agent_state.travel_distance -= 1;
    return new_direction;
  }

  bool within_site(int x, int y)
  {
    return x >= 0 && x < HEIGHT && y >= 0 && y < WIDTH;
  }

  Location *loc;
  AgentState state;
  parlay::random_generator rgen;
};

using Agent = AgentTemplate<TaskAllocLocationState, TaskAllocAgentState>;