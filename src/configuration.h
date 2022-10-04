#include "agent.h"
#include "map.h"
#include "states.h"
#include "parlay/sequence.h"

class Configuration {
public:
  Configuration(int n_, int m_, bool torus_)
  : n(n_)
  , m(m_)
  , torus(torus_)
  , map(n_, m_) 
  { }

  void add_agents(parlay::sequence<Position> &agent_pos);
  void reset_agents(parlay::sequence<Position> &agent_pos);
  void set_task_vertex(Position &pos);
  bool execute_transition();
  void transition();
  bool all_agents_terminated();
  bool all_tasks_completed();

  Location* get_vertex(int x, int y);
  Location* get_task(int i);

  BasicMap map;

private:
  int n;
  int m;
  bool torus;
  parlay::sequence<Position> task_vertices;
  parlay::sequence<Agent> agents;
};