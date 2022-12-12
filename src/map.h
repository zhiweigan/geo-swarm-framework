#pragma once
#include <string>
#include <vector>
#include "states.h"

class BasicMap
{
public:
  BasicMap(int16_t n_, int16_t m_) 
  : n(n_)
  , m(m_)
  { 
    for (int i = 0; i < n; i++) {
      for(int j = 0; j < m; j++) {
        vertices.push_back(Location(i, j));
        transitions.push_back(LocalTransitory{{Position{static_cast<int16_t>(i), static_cast<int16_t>(j)}}, {}});
        messages.push_back({});
        messages[i * m + j].reserve(5);
      }
    }
  }

  Location *get_vertex(int x, int y);
  void set_transition(int x, int y, LocalTransitory &&transition);
  LocalTransitory *get_transition(int x, int y);

#ifdef MESSAGE
  void add_message(int x, int y, AgentMessage &&message);
  std::vector<AgentMessage> *get_messages(int x, int y);
#endif

  bool in_bounds(int x, int y);
  
  int16_t n;
  int16_t m;

private:
  std::vector<Location> vertices;
  std::vector<std::vector<AgentMessage>> messages;
  std::vector<LocalTransitory> transitions;
};
