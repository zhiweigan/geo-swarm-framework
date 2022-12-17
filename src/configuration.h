#include "agent.h"
#include "map.h"
#include "states.h"
#include "constants.h"
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
  void transition();
  Location *get_vertex(int x, int y);
  void parallel_setup();

  void custom_setup();
  void update_agents();
  void update_locations();
  bool is_finished();
  void print_config(int time = 0, int flags = 0);

  // TODO: REMOVE
  void set_task_vertex(Position &pos);
  Location *get_task(int i);
  parlay::sequence<Position> *get_tasks();
  size_t num_unique_locations;

  int n;
  int m;
  int rounds = 0;
  BasicMap map;

  // benchmarking
  uint64_t sorting = 0;
  uint64_t transitions = 0;
  uint64_t update = 0;
  uint64_t messages = 0;
  uint64_t parallel = 0;

  parlay::sequence<Agent> agents;

private:
  int influence_radius = 2;
  
  
  
  parlay::sequence<AgentTransition> agent_transitions;
  parlay::sequence<AgentMessage> agent_messages;

  std::map<Position, LocalMapping> local_mappings;
  parlay::sequence<Position> task_vertices;

  // parallel bookkeeping
  parlay::sequence<int> loc_diff;
  parlay::sequence<int> is_diff;
  parlay::sequence<int> agent_offset;
  parlay::sequence<int> individual_agent;
  parlay::sequence<int> offsets;
  parlay::sequence<int> counts;
  parlay::sequence<Location*> unique_vertices;

  parlay::sequence<int> agent_ids;
  parlay::sequence<int> removed_agent_ids;
};