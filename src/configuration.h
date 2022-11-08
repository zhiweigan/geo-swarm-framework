#include "agent.h"
#include "map.h"
#include "states.h"
#include "constants.h"
#include "res_utils.h"
#include "parlay/sequence.h"
#include <chrono>

class Configuration {
public:
  Configuration(int n_, int m_)
  : n(n_)
  , m(m_)
  , map(n_, m_) 
  { }

  void init();
  void add_agents(std::vector<Position> &agent_pos);
  void reset_agents(std::vector<Position> &agent_pos);
  void set_task_vertex(Position &pos);
  void execute_transition();
  void transition();
  bool all_agents_terminated();
  bool all_tasks_completed();

  void generate_global_transitory();

  void print_config(int time = 0);

  Location* get_vertex(int x, int y);
  Location* get_task(int i);

  int n;
  int m;
  BasicMap map;

  // benchmarking
  int sorting = 0;
  int transitions = 0;
  int update = 0;

private:
  int influence_radius = 2;
  std::vector<Position> task_vertices;
  parlay::sequence<Agent> agents;
  
  parlay::sequence<AgentTransition> agent_transitions;
  std::map<Position, LocalMapping> local_mappings;


  // parallel bookkeeping
  parlay::sequence<int> loc_diff;
  parlay::sequence<int> is_diff;
  parlay::sequence<int> agent_offset;
  parlay::sequence<int> individual_agent;
  parlay::sequence<int> offsets;
  parlay::sequence<int> counts;
  parlay::sequence<Location*> unique_vertices;

  parlay::sequence<Agent *> *agent_ptrs;
  parlay::sequence<Agent *> *agent_ptrs_tmp;
};