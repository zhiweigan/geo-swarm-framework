#include "map.h"
#include "geo_utils.h"
#include "parlay/random.h"
#include "configuration.h"
#include "parlay/sequence.h"

using namespace parlay;

parlay::random_generator rgen = parlay::random_generator();

// run a small test case
int
main()
{
  // constants
  int n = 5, m = 5;
  bool torus = false;
  int num_agents = 3;
  int num_tasks = 2;
  int total_demand = 3;
  Position home_loc{1,2};

  Configuration config(n, m, torus);
  config.get_vertex(home_loc.x, home_loc.y)->state = LocationState(Position{home_loc.x, home_loc.y}, false, true);

  parlay::sequence<LocationState> tasks;
  for (int i = 0; i < num_tasks; i++) {
    Position task_loc{(int16_t)(rand() % n), (int16_t)(rand() % m)};
    while (task_loc == home_loc) {
      task_loc = Position{(int16_t)(rand() % n), (int16_t)(rand() % m)};
    }
    config.set_task_vertex(task_loc);
    tasks.emplace_back(task_loc, true, false, 1, 1);
  }

  for (int i = 0; i < total_demand - num_tasks; i++) {
    int task_num = rand() % num_tasks;
    tasks[task_num].demand += 1;
    tasks[task_num].residual_demand += 1;
  }
 
  for (auto task : tasks) {
    config.map.get_vertex(task.task_location.x, task.task_location.y)->state = std::move(task);
  }

  parlay::sequence<Position> agent_pos;
  for (int i = 0; i < num_agents; i++) {
    agent_pos.emplace_back(home_loc.x, home_loc.y);
  }
  config.add_agents(agent_pos);

  config.print_config();

  int total_rd = total_demand;
  while (total_rd > 0) {
    config.transition();
    total_rd = 0;
    for(int i = 0; i < num_tasks; i++) {
      total_rd += config.get_task(i)->state.residual_demand;
    }
    config.print_config();
    break;
  }

  return 0;
}