#pragma once
#include <string>
#include <vector>
#include <states.h>

class BasicMap
{
public:
  BasicMap(int16_t n_, int16_t m_, int16_t o_) 
  : n(n_)
  , m(m_)
  , o(o_)
  { 
    for (int i = 0; i < n; i++) {
      for(int j = 0; j < m; j++) {
        for (int k = 0; k < o; k++) {
          vertices.push_back(Location(i, j, k));
          messages.push_back({});
          messages[i * m * o + j * o + k].reserve(5);
        }
      }
    }
  }

  Location *get_vertex(int x, int y, int z);

  void add_message(int x, int y, int z, AgentMessage &&message);
  std::vector<AgentMessage> *get_messages(int x, int y, int z);

  bool in_bounds(int x, int y, int z);
  
  int16_t n;
  int16_t m;
  int16_t o;

private:
  std::vector<Location> vertices;
  std::vector<std::vector<AgentMessage>> messages;
};
