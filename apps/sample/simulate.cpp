#include <chrono>

#include "../../src/map.h"
#include "../../src/geo_utils.h"
#include "../../src/configuration.h"
#include "../../src/parlay/primitives.h"

// run a small test case
int
main()
{
  // TODO: optimize setup
  // constants
  bool verbose = false;
  int n = HEIGHT, m = WIDTH;
  int num_agents = NUM_AGENTS;

  Configuration config(n, m);
  config.init();

  std::vector<Position> agent_pos;
  for (int i = 0; i < num_agents; i++) {
    agent_pos.emplace_back(rand() % n, rand() % m);
  }
  config.add_agents(agent_pos);
  config.parallel_setup();

  int iter = 0;
  std::cout<<"Starting simulation"<<std::endl;

  while (!config.is_finished()) {
    config.transition();
    iter++;

    if (iter > n / 2)
      break;

    if (ITERS > 0 && iter >= ITERS)
      break;
    if (verbose && iter % 100 == 0) {
      std::cout << "Iteration: " << iter <<std::endl;
    }
  }

  config.print_config();

  return 0;
}