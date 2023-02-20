#include <agents.h>
#include "map.h"
#include <states.h>
#include <constants.h>
#include <parlay/sequence.h>
#include <chrono>


class Configuration {
public:
  Configuration(int n_, int m_, int inf_ = 2)
  : n(n_)
  , m(m_)
  , influence_radius(inf_)
  , map(n_, m_) 
  { }
  ~Configuration();

  void init();
  void add_agents(std::vector<Position> &agent_pos);
  void transition();
  Location *get_vertex(int x, int y);
  void parallel_setup();

  struct UserDefined;
  UserDefined* custom = 0;
  void custom_setup();

  void update_agent(int i);
  // void update_location(Location * loc, );
  bool is_finished();
  void print_config(int time = 0, int flags = 0);
  parlay::slice<> get_agents_at_loc(Location* loc);

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