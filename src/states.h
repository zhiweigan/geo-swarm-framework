#pragma once
#include <cstdint>
#include <string>
#include <set>

struct Position {
  int16_t x;
  int16_t y;

  Position(int16_t x_, int16_t y_)
    : x(x_)
    , y(y_) 
  { }

  bool operator==(Position const &rhs) const 
  { 
    return x == rhs.x && y == rhs.y; 
  }

  bool operator!=(Position const &rhs) const
  {
    return x != rhs.x || y != rhs.y;
  }
};

struct LocationState
{
  LocationState(
    Position task_location_,
    bool is_task_ = false,
    bool is_home_ = false,
    int16_t demand_ = 0,
    int16_t residual_demand_ = 0
  )
    : is_task(is_task_)
    , is_home(is_home_)
    , demand(demand_)
    , residual_demand(residual_demand_)
    , task_location(task_location_)
  { }
  bool is_task;
  bool is_home;
  int16_t demand;
  int16_t residual_demand;
  Position task_location;
};

class Location
{
public:
  Location(int16_t x_, int16_t y_)
    : loc(Position(x_, y_))
    , state(LocationState(Position{x_, y_}))
  { }

  Position loc;
  LocationState state;
  std::set<int16_t> agents_seen;
};

struct AgentState
{
  AgentState(int16_t id_, Location &v_, double l_) 
  : core_state("None")
  , id(id_)
  , levy_cap(l_)
  , angle(0.0)
  , travel_distance(0)
  , starting_point(Position{v_.loc.x, v_.loc.y})
  { }

  std::string core_state;
  int16_t id;
  double levy_cap;
  double angle;
  int travel_distance;
  Position starting_point;
};