#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>

enum Direction {
  S, L, R, D, U, LAST
};

struct Position {
  int16_t x;
  int16_t y;

  Position(int16_t x_ = 0, int16_t y_ = 0)
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

  bool operator<(Position const &rhs) const
  {
    if (x < rhs.x) return true;
    if (rhs.x < x) return false;
    if (y < rhs.y) return true;
    if (rhs.y < y) return false;
    return false;
  }

  friend std::ostream &operator<<(std::ostream &os, const Position &m)
  {
    os << "("<<m.x << ","<<m.y<<")";
    return os;
  }
};

struct TaskAllocLocationState
{
  TaskAllocLocationState(
    Position task_location_,
    bool is_task_ = false,
    bool is_home_ = false,
    int demand_ = 0,
    int residual_demand_ = 0
  )
    : is_task(is_task_)
    , is_home(is_home_)
    , demand(demand_)
    , residual_demand(residual_demand_)
    , task_location(task_location_)
  { }
  bool is_task;
  bool is_home;
  int demand;
  int residual_demand;
  Position task_location;
};

template<class LocationState, class AgentState>
class LocationTemplate
{
public:
  LocationTemplate(int16_t x_, int16_t y_)
    : loc(Position(x_, y_))
    , state(LocationState(Position{x_, y_}))
  { }

  Position loc;
  LocationState state;
  std::set<int> agents_seen;
};

struct TaskAllocAgentState
{
  TaskAllocAgentState() {}
  TaskAllocAgentState(int id_, LocationTemplate<TaskAllocLocationState, TaskAllocAgentState> &v_, double l_) 
  : core_state("None")
  , id(id_)
  , levy_cap(l_)
  , angle(0.0)
  , travel_distance(0)
  , starting_point(Position{v_.loc.x, v_.loc.y})
  { }

  std::string core_state;
  int id;
  double levy_cap;
  double angle;
  int travel_distance;
  Position starting_point;
  LocationTemplate<TaskAllocLocationState, TaskAllocAgentState> *committed_task = nullptr;
  LocationTemplate<TaskAllocLocationState, TaskAllocAgentState> *destination_task = nullptr;
};

template <class AgentState>
struct ProposedAgentTransitionTemplate
{
  AgentState astate;
  Direction dir;
};

template <class LocationState, class AgentState>
struct LocalTransitoryTemplate
{
  LocationState loc_state;
  std::map<int, ProposedAgentTransitionTemplate<AgentState>> agent_updates;
};

template <class LocationState, class AgentState>
struct AgentTransitionTemplate
{
  AgentTransitionTemplate (
    LocationState lstate_,
    AgentState astate_
  )
  : lstate (lstate_)
  , astate (astate_)
  , dir (Direction::S)
  { }

  AgentTransitionTemplate(
    LocationState lstate_,
    AgentState astate_,
    Direction dir_
  )
  : lstate(lstate_)
  , astate(astate_)
  , dir(dir_)
  { }

  LocationState lstate;
  AgentState astate;
  Direction dir;
};

using Location = LocationTemplate<TaskAllocLocationState, TaskAllocAgentState>;
using ProposedAgentTransition = ProposedAgentTransitionTemplate <TaskAllocAgentState>;
using LocalTransitory = LocalTransitoryTemplate<TaskAllocLocationState, TaskAllocAgentState>;
using AgentTransition = AgentTransitionTemplate<TaskAllocLocationState, TaskAllocAgentState>;
typedef std::map<Position, Location *> LocalMapping;
