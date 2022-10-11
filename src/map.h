#pragma once
#include <string>
#include <vector>
#include "states.h"

// TODO: defines an abstract class that is implemented by all map types
// class Map {
// public:
//   virtual Location *get_vertex(int x, int y) = 0;
// };

class BasicMap {
public:
  BasicMap(int16_t n_, int16_t m_) 
  : n(n_)
  , m(m_)
  { 
    for (int i = 0; i < n; i++) {
      for(int j = 0; j < m; j++) {
        vertices.push_back(Location(i, j));
      }
    }
  } 

  Location *get_vertex(int x, int y);
  bool in_bounds(int x, int y);
  
  int16_t n;
  int16_t m;

private:
  std::vector<Location> vertices;
};


