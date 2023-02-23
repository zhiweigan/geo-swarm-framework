#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <set>
#include <map>
#include <vector>

enum Direction {
  S, L, R, D, U, I, O, LAST
};

struct Position {
  int16_t x;
  int16_t y;
  int16_t z;

  Position(int16_t x_ = 0, int16_t y_ = 0, int16_t z_ = 0)
    : x(x_)
    , y(y_) 
    , z(z_)
  { }

  bool operator==(Position const &rhs) const 
  {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }

  bool operator!=(Position const &rhs) const
  {
    return x != rhs.x || y != rhs.y || y != rhs.y;
  }

  bool operator<(Position const &rhs) const
  {
    if (x < rhs.x) return true;
    if (rhs.x < x) return false;
    if (y < rhs.y) return true;
    if (rhs.y < y) return false;
    if (z < rhs.z) return true;
    if (rhs.z < z) return false;
    return false;
  }

  friend std::ostream &operator<<(std::ostream &os, const Position &m)
  {
    os << "("<<m.x << ","<<m.y<<","<<m.z<<")";
    return os;
  }
};

template<class LocationState>
class LocationTemplate
{
public:
  LocationTemplate(int16_t x_, int16_t y_, int16_t z_)
    : loc(Position(x_, y_, z_))
    , state(LocationState(Position{x_, y_, z_}))
  { }

  Position loc;
  LocationState state;
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