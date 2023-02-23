#include <chrono>

#include <map.h>
#include <geo_utils.h>
#include <configuration.h>
#include <parlay/primitives.h>

// run a small test case
int
main()
{
  // constants
  bool verbose = false;
  int n = HEIGHT, m = WIDTH, o = DEPTH;
  int num_agents = NUM_AGENTS;

  Configuration config(n, m, o);
  config.init();

  std::vector<Position> agent_pos;
  for (int i = 0; i < num_agents; i++) {
    agent_pos.emplace_back(rand() % n, rand() % m, rand() % o);
  }
  config.add_agents(agent_pos);
  config.parallel_setup();

  int iter = 0;
  std::cout<<"Starting simulation"<<std::endl;

  while (!config.is_finished()) {
    config.transition();
    iter++;

    if (MAX_ITERS > 0 && iter >= MAX_ITERS)
      break;
    if (verbose && iter % 100 == 0) {
      std::cout << "Iteration: " << iter <<std::endl;
    }
  }

  config.print_config();

  return 0;
}