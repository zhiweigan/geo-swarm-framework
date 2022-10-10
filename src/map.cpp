#include "map.h"

Location* BasicMap::get_vertex(int x, int y)
{
  return &vertices[x * m + y];
}

bool BasicMap::in_bounds(int x, int y)
{
  return x >= 0 && x < n && y >= 0 && y < m;
}