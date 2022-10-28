#include "map.h"

Location* BasicMap::get_vertex(int x, int y)
{
  return &vertices[x * m + y];
}

void BasicMap::set_transition(int x, int y, LocalTransitory &&transition){
  transitions[x * m + y] = std::move(transition);
}

LocalTransitory *BasicMap::get_transition(int x, int y)
{
  return &transitions[x * m + y];
}

bool BasicMap::in_bounds(int x, int y)
{
  return x >= 0 && x < n && y >= 0 && y < m;
}