#pragma once
#include <states.h>
#include <map.h>
#include <constants.h>
#include <parlay/random.h>
#include <math.h>
#include <geo_utils.h>
#include <map>
#include <cstdint>
#include <chrono>
#include <climits>

int16_t dx[8] = {0,1,1, 1,-1,-1, 0,-1};
int16_t dy[8] = {1,1,0,-1, 0, 1,-1,-1};

template <class LocationState, class AgentState>
class AgentTemplate
{
public:
  AgentTemplate(int id_, Location &loc_) 
  : loc(&loc_)
  , state(AgentState(id_))
  , rgen(std::chrono::system_clock::now().time_since_epoch().count())
  , percentage(1.0)
  , active(true)
  { }

  bool onWall(Location* loc)
  {
    return loc->state.isWall;
  }
  
  bool inBounds(Location* loc)
  {
    return loc->loc.x >= 0 && loc->loc.x < HEIGHT && loc->loc.y >= 0 && loc->loc.y < WIDTH;
  }

  float waitfn(float s)
  {
    return WMAX * (s*s) / (s*s + RESPONSIVE);
  }

  AgentTransition generate_transition(LocalMapping &local_mapping)
  {
    AgentState new_astate = state;

    if (new_astate.wait_left > 0)
    {
      new_astate.wait_left -= 1;
      return AgentTransition(loc->state, std::move(new_astate), Direction::S);
    }

    Direction new_direction = get_direction_from_angle(new_astate.angle, new_astate.starting_point, Position{loc->loc.x, loc->loc.y});
    Position new_location = get_coords_from_movement(Position{loc->loc.x, loc->loc.y}, new_direction, true);

    // start state
    if (new_astate.angle == -1) {
      do 
      {
        new_astate.angle = static_cast<double>(rgen()) / (static_cast<double>(rgen.max()) / (2.0 * M_PI));
        new_astate.starting_point = Position{loc->loc.x, loc->loc.y};
        new_direction = get_direction_from_angle(new_astate.angle, new_astate.starting_point, Position{loc->loc.x, loc->loc.y});
        new_location = get_coords_from_movement(Position{loc->loc.x, loc->loc.y}, new_direction, true);
      } while((!inBounds(local_mapping[get_delta_from_movement(new_direction)].first)));
    }

    bool collision = local_mapping[get_delta_from_movement(new_direction)].second->size() > 0;
    if (collision)
    {
      Direction new_dir = Direction::S;
      double heat = local_mapping[{0,0}].first->state.heat;
      if (heat > 0)
      {
        new_astate.angle = -1;
        new_astate.wait_left = waitfn(heat);

        // if (heat > 0.05) 
        // {
        //   if (local_mapping[{0,1}].second->size() > 0 && local_mapping[{0,1}].first->state.heat > heat) new_dir = Direction::R;
        //   if (local_mapping[{0,-1}].second->size() > 0 && local_mapping[{0,-1}].first->state.heat > heat) new_dir = Direction::L;
        //   if (local_mapping[{1,0}].second->size() > 0 && local_mapping[{1,0}].first->state.heat > heat) new_dir = Direction::D;
        //   if (local_mapping[{-1,0}].second->size() > 0 && local_mapping[{-1,0}].first->state.heat > heat) new_dir = Direction::U;
        // }
      }
      else
      {
        new_astate.angle = -1;
      }
      return AgentTransition(loc->state, std::move(new_astate), new_dir);
    }
    if (onWall(local_mapping[{0,0}].first) && !inBounds(local_mapping[get_delta_from_movement(new_direction)].first))
    {
      new_astate.angle = -1;
      return AgentTransition(loc->state, std::move(new_astate), Direction::S);
    }

    return AgentTransition(loc->state, std::move(new_astate), new_direction);
  }

  AgentMessage generate_message(LocalMapping &local_mapping)
  {
    return AggregateAgentMessage({0, local_mapping[{0,0}].first->state.heat});
  }

  Location *loc;
  AgentState state;
  parlay::random_generator rgen;
  bool active;
  int percentage;
};

using Agent = AgentTemplate<AggregateLocationState, AggregateAgentState>;