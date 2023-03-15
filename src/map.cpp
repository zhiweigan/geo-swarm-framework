#include "map.h"

#ifndef DIM3
Location *Torus2D::get_vertex(int x, int y)
{
  return &vertices[x * m + y];
}

bool Torus2D::in_bounds(int x, int y)
{
  return x >= 0 && x < n && y >= 0 && y < m;
}

void Torus2D::add_message(int x, int y, AgentMessage &&message)
{
  messages[x * m + y].push_back(std::move(message));
}

std::vector<AgentMessage> *Torus2D::get_messages(Position pos)
{
  return &messages[pos.x * m + pos.z];
}

#else 

Location *Torus3D::get_vertex(int x, int y, int z)
{
  return &vertices[x * m * o + y * o + z];
}

bool Torus3D::in_bounds(int x, int y, int z)
{
  return x >= 0 && x < n && y >= 0 && y < m && z >= 0 && z < o;
}

void Torus3D::add_message(int x, int y, int z, AgentMessage &&message)
{
  messages[x * m * o + y * o + z].push_back(std::move(message));
}

std::vector<AgentMessage> *Torus3D::get_messages(Position pos)
{
  return &messages[pos.x * m * o + pos.y * o + pos.z];
}

#endif