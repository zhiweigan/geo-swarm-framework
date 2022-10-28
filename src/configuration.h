#include "agent.h"
#include "map.h"
#include "states.h"
#include "constants.h"
#include "res_utils.h"

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
  void delta(std::map<Position, Location *> &local_mapping);

  void print_config(int time = 0);

  Location* get_vertex(int x, int y);
  Location* get_task(int i);

  int n;
  int m;
  BasicMap map;

private:
  int influence_radius = 2;
  std::vector<Position> task_vertices;
  std::vector<Agent> agents;
  std::map<Position, LocalMapping> local_mappings;
};