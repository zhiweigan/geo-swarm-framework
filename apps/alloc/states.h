#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include "../template_states.h"

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

struct TaskAllocAgentState
{
  TaskAllocAgentState(int id_): id(id_) {}
  TaskAllocAgentState(int id_, LocationTemplate<TaskAllocLocationState> &v_, double l_) 
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
  LocationTemplate<TaskAllocLocationState> *committed_task = nullptr;
  LocationTemplate<TaskAllocLocationState> *destination_task = nullptr;
};

struct AllocAgentMessage
{

};

using Location                = LocationTemplate<TaskAllocLocationState>;
using AgentTransition         = AgentTransitionTemplate<TaskAllocLocationState, TaskAllocAgentState>;
using AgentMessage            = AllocAgentMessage;
using LocalMapping            = std::map<Position, std::pair<Location *, std::vector<AgentMessage> *>>;
