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
  , accepted(false)
  { }

  AgentTransitionTemplate(
    LocationState lstate_,
    AgentState astate_,
    Direction dir_
  )
  : lstate(lstate_)
  , astate(astate_)
  , dir(dir_)
  , accepted(false)
  { }

  LocationState lstate;
  AgentState astate;
  Direction dir;
  bool accepted;
};