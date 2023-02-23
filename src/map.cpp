#include "map.h"

Location *BasicMap::get_vertex(int x, int y, int z)
{
  return &vertices[x * m * o + y * o + z];
}

bool BasicMap::in_bounds(int x, int y, int z)
{
  return x >= 0 && x < n && y >= 0 && y < m && z >= 0 && z < o;
}

void BasicMap::add_message(int x, int y, int z, AgentMessage &&message)
{
  messages[x * m * o + y * o + z].push_back(std::move(message));
}

std::vector<AgentMessage> *BasicMap::get_messages(int x, int y, int z)
{
  return &messages[x * m * o + y * o + z];
}
