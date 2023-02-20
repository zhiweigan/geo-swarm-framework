#include "map.h"

Location *BasicMap::get_vertex(int x, int y)
{
  return &vertices[x * m + y];
}

bool BasicMap::in_bounds(int x, int y)
{
  return x >= 0 && x < n && y >= 0 && y < m;
}

void BasicMap::add_message(int x, int y, AgentMessage &&message)
{
  messages[x * m + y].push_back(std::move(message));
}

std::vector<AgentMessage> *BasicMap::get_messages(int x, int y)
{
  return &messages[x * m + y];
}
