#include <agents.h>
#include "map.h"
#include <states.h>
#include <constants.h>
#include <functional>
#include <parlay/sequence.h>
#include <chrono>

template<class Map>
class Configuration {
public:
  Configuration(Map map_, int inf_ = 2)
  : influence_radius(inf_)
  , map(map_) 
  { }
  ~Configuration();

  void init();
  void transition();
  void parallel_setup();
  void add_agent(Agent agent);

  struct UserDefined;
  UserDefined* custom = 0;
  void custom_setup();

  // ------------- HELPERS -------------

  parlay::slice<int *, int *> getAgentsAtUniqueLocation(int i);
  parlay::slice<AgentTransition *, AgentTransition *> getAgentTransitionsAtUniqueLocation(int i);
  parlay::slice<int *, int *> getAgentsNextToAgent(int i);
  void loopOverAgents(parlay::slice<int *, int *> agents, const std::function<void(int)> &f);

  // -----------------------------------

  void update_agent(int i);
  void update_location(Location *loc, parlay::slice<int *, int *> agents, parlay::slice<AgentTransition *, AgentTransition *> transitions);
  bool is_finished();
  void print_config(int time = 0, int flags = 0);

  int rounds = 0;
  Map map;

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
  parlay::sequence<int> filter_space;
  parlay::sequence<int> counts;
  parlay::sequence<Location*> unique_vertices;
  size_t num_unique_locations;

  parlay::sequence<int> agent_ids;
  parlay::sequence<int> removed_agent_ids;
};

#include <configuration.hpp>
#include <appconfig.hpp>