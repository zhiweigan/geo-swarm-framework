#pragma once
#include <map>
#include <cmath>
#include <math.h>

#include <string>
#include "map.h"
#include <states.h>
#include <constants.h>

#ifndef DIM3
inline std::map<Position, std::pair<Location *, std::vector<AgentMessage>*>> generate_local_mapping(
  Location &vtx, 
  int influence_radius, 
  Torus2D &map)
{
  std::map<Position, std::pair<Location *, std::vector<AgentMessage> *>> ret_map;
  int vx = vtx.loc.x;
  int vy = vtx.loc.y;

  for (int16_t dx = -influence_radius; dx <= influence_radius; dx++)
  {
    for (int16_t dy = -influence_radius; dy <= influence_radius; dy++)
    {
      int16_t nvx = ((vx + dx) % HEIGHT + HEIGHT) % HEIGHT;
      int16_t nvy = ((vy + dy) % WIDTH + WIDTH) % WIDTH;
      ret_map.insert(std::make_pair(Position{dx, dy}, std::make_pair(map.get_vertex(nvx, nvy), map.get_messages(Position{nvx, nvy}))));
    }
  }
  return ret_map;
}
#else
inline std::map<Position, std::pair<Location *, std::vector<AgentMessage>*>> generate_local_mapping(
  Location &vtx, 
  int influence_radius, 
  Torus3D &map)
{
  std::map<Position, std::pair<Location *, std::vector<AgentMessage> *>> ret_map;
  int vx = vtx.loc.x;
  int vy = vtx.loc.y;
  int vz = vtx.loc.z;

  for (int16_t dx = -influence_radius; dx <= influence_radius; dx++)
  {
    for (int16_t dy = -influence_radius; dy <= influence_radius; dy++)
    {
      for (int16_t dz = -influence_radius; dz <= influence_radius; dz++)
      {
        int16_t nvx = ((vx + dx) % HEIGHT + HEIGHT) % HEIGHT;
        int16_t nvy = ((vy + dy) % WIDTH + WIDTH) % WIDTH;
        int16_t nvz = ((vz + dz) % DEPTH + DEPTH) % DEPTH;
        ret_map.insert(std::make_pair(Position{dx, dy, dz}, std::make_pair(map.get_vertex(nvx, nvy, nvz), map.get_messages(Position{nvx, nvy, nvz}))));
      }
    }
  }
  return ret_map;
}
#endif

inline int mod(int a, int b)
{
  return (a % b + b) % b;
}

inline Position get_coords_from_movement(Position pos, Direction dir, bool ignore_bound = false)
{
  switch(dir) {
    case Direction::S:
      return pos;
    case Direction::L:
      pos.x = mod(pos.x - 1, HEIGHT);
      return pos;
    case Direction::R:
      pos.x = mod(pos.x + 1, HEIGHT);
      return pos;
    case Direction::D:
      pos.y = mod(pos.y - 1, WIDTH);
      return pos;
    case Direction::U:
      pos.y = mod(pos.y + 1, WIDTH);
      return pos;

#ifdef DIM3
    case Direction::I:
      pos.y = mod(pos.z - 1, DEPTH);
      return pos;
    case Direction::O:
      pos.y = mod(pos.z + 1, DEPTH);
      return pos;
#endif

    case Direction::LAST:
      return pos;
  } 
  return pos;
}

inline double get_slope(int x1, int y1, int x2, int y2)
{
  if (x1 == x2)
    return tan(M_PI / 2);
  else
    return ((double)(y1-y2)) / ((double)(x1-x2));
}

template <typename T>
inline int sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}

inline Direction get_direction_from_angle(double angle, Position start, Position curr_loc)
{
  int16_t xdir = sgn(cos(angle));
  int16_t ydir = sgn(sin(angle));

  Position move_x{static_cast<int16_t>(curr_loc.x + xdir), curr_loc.y};
  Position move_y{curr_loc.x, static_cast<int16_t>(curr_loc.y + ydir)};

  double slope = tan(angle);
  double x_closeness = abs(get_slope(start.x, start.y, move_x.x, move_x.x) - slope);
  double y_closeness = abs(get_slope(start.x, start.y, move_y.y, move_y.y) - slope);
  
  if ((x_closeness < y_closeness || (x_closeness == y_closeness && rand() % 2 == 0)) && xdir != 0) 
  {
    if (xdir > 0) return Direction::R;
    else return Direction::L;
  }
  else
  {
    if (ydir > 0) return Direction::U;
    else return Direction::U;
  }
}

inline double l2_distance(int x1, int y1, int x2, int y2)
{
  return sqrt((double)pow(x1-x2, 2) + (double)pow(y1-y2, 2));
}

inline Direction get_direction_from_destination(Position dest, Position curr)
{
  double best_dist = l2_distance(dest.x, dest.y, curr.x, curr.y);
  Direction best_dir = Direction::S;
  for (int dirIter = Direction::S; dirIter != Direction::LAST; dirIter++)
  {
    Direction dir = static_cast<Direction>(dirIter);
    Position new_loc = get_coords_from_movement(curr, dir);
    double new_dist = l2_distance(new_loc.x, new_loc.y, dest.x, dest.y);
    if (new_dist < best_dist) 
    {
      best_dist = new_dist;
      best_dir = dir;
    }
  }
  return best_dir;
}