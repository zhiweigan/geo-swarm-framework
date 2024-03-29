#pragma once
#include <string>
#include <vector>
#include <states.h>

#ifndef DIM3
class Torus2D
{
public:
  Torus2D(int16_t n_, int16_t m_) 
  : n(n_)
  , m(m_)
  { 
    for (int i = 0; i < n; i++) {
      for(int j = 0; j < m; j++) {
        vertices.push_back(Location(i, j));
        messages.push_back({});
        messages[i * m + j].reserve(5);
      }
    }
  }

  Location *get_vertex(int x, int y);

  void add_message(int x, int y, AgentMessage &&message);
  std::vector<AgentMessage> *get_messages(Position pos);

  bool in_bounds(int x, int y);
  
  int16_t n;
  int16_t m;

private:
  std::vector<Location> vertices;
  std::vector<std::vector<AgentMessage>> messages;
};

#else

class Torus3D
{
public:
  Torus3D(int16_t n_, int16_t m_, int16_t o_) 
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
  std::vector<AgentMessage> *get_messages(Position pos);

  bool in_bounds(int x, int y, int z);
  
  int16_t n;
  int16_t m;
  int16_t o;

private:
  std::vector<Location> vertices;
  std::vector<std::vector<AgentMessage>> messages;
};

#endif