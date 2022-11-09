#include <chrono>

#include "map.h"
#include "geo_utils.h"
#include "configuration.h"

// run a small test case
int
main()
{
  // constants
  bool verbose = false;
  int n = HEIGHT, m = WIDTH;
  int num_agents = NUM_AGENTS;
  int num_tasks = NUM_TASKS;
  int total_demand = TOTAL_DEMAND;
  Position home_loc{1,2};
  srand(0);

  Configuration config(n, m);
  config.init();
  config.get_vertex(home_loc.x, home_loc.y)->state = LocationState(Position{home_loc.x, home_loc.y}, false, true);

  std::vector<LocationState> tasks;
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

  std::vector<Position> agent_pos;
  for (int i = 0; i < num_agents; i++) {
    agent_pos.emplace_back(home_loc.x, home_loc.y);
  }
  config.add_agents(agent_pos);

  int total_rd = total_demand;
  int iter = 0;
  int time_checking = 0;
  std::cout<<"Starting simulation"<<std::endl;
  auto t1 = std::chrono::high_resolution_clock::now();
  while (total_rd > 0) {
    config.transition();
    total_rd = 0;
    auto check_start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < num_tasks; i++) { // make this parallel with reduce
      total_rd += config.get_task(i)->state.residual_demand; // abstract out termination criteria
    }
    iter++;
    if (verbose && iter % 100 == 0)
    {
      std::cout<<"Iteration "<<iter<<std::endl;
    }
    auto check_end = std::chrono::high_resolution_clock::now();
    time_checking += std::chrono::duration_cast<std::chrono::nanoseconds>(check_end - check_start).count();
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "checking took "
            << time_checking / 1e6
            << " milliseconds \n";

  std::cout << "sorting took "
            << config.sorting / 1e6
            << " milliseconds \n";

  std::cout << "generating transitions took "
            << config.transitions / 1e6
            << " milliseconds \n";

  std::cout << "updating took "
            << config.update / 1e6
            << " milliseconds \n";

  std::cout << "simulation took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
            << " milliseconds\n";

  std::cout << "simulation completed in "
            << iter
            << " iterations\n";
  // TOOD: make this take command line arguments for constants

  return 0;
}