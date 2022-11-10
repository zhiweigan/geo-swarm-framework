#include <chrono>

#include "map.h"
#include "geo_utils.h"
#include "configuration.h"

#include "parlay/primitives.h"

// run a small test case
int
main()
{
  // TODO: optimize setup
  // constants
  bool verbose = VERBOSE;
  int n = HEIGHT, m = WIDTH;
  int num_agents = NUM_AGENTS;
  int num_tasks = NUM_TASKS;
  int total_demand = TOTAL_DEMAND;
  Position home_loc{1,2};

  Configuration config(n, m);
  config.init();
  config.get_vertex(home_loc.x, home_loc.y)->state = LocationState(Position{home_loc.x, home_loc.y}, false, true);

  std::vector<LocationState> tasks;
  for (int i = 0; i < num_tasks; i++) {
    Position task_loc{(int16_t)(rand() % n), (int16_t)(rand() % m)};
    while (task_loc == home_loc || config.map.get_vertex(task_loc.x, task_loc.y)->state.is_task)
    {
      task_loc = Position{(int16_t)(rand() % n), (int16_t)(rand() % m)};
    }
    tasks.emplace_back(task_loc, true, false, 1, 1);
    config.set_task_vertex(task_loc);
    config.map.get_vertex(task_loc.x, task_loc.y)->state.is_task = true;
  }

  for (int i = 0; i < total_demand - num_tasks; i++) {
    int task_num = rand() % num_tasks;
    tasks[task_num].demand += 1;
    tasks[task_num].residual_demand += 1;
  }
 
  for (auto task : tasks) {
    config.map.get_vertex(task.task_location.x, task.task_location.y)->state = task;
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
  auto get_demand = [&] (Position task_vertex) { return config.get_vertex(task_vertex.x, task_vertex.y)->state.residual_demand; };

  while (total_rd > 0) {
    config.transition();
    auto check_start = std::chrono::high_resolution_clock::now();
    total_rd = parlay::reduce(parlay::delayed_map(*config.get_tasks(), get_demand));
    iter++;
    auto check_end = std::chrono::high_resolution_clock::now();
    time_checking += std::chrono::duration_cast<std::chrono::nanoseconds>(check_end - check_start).count();

    if (ITERS > 0 && iter >= ITERS)
      break;
    if (verbose && iter % 100 == 0) {
      std::cout << "Iteration: " << iter << ", Residual Demand: "<<total_rd<<std::endl;
      // config.print_config();
    }
  }

  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "checking took "
            << time_checking / 1e6
            << " milliseconds\n";

  std::cout << "sorting took "
            << config.sorting / 1e6
            << " milliseconds\n";

  std::cout << "generating transitions took "
            << config.transitions / 1e6
            << " milliseconds\n";

  std::cout << "updating took "
            << config.update / 1e6
            << " milliseconds\n";

  std::cout << "simulation completed in "
            << iter
            << " iterations\n";

  std::cout << "simulation took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
            << " milliseconds\n";

  return 0;
}