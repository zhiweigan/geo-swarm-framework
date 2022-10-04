#include "map.h"

Location* BasicMap::get_vertex(int x, int y)
{
  return &vertices[x * m + y];
}