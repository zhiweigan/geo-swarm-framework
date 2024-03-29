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
  int n = HEIGHT, m = WIDTH;
  int num_agents = NUM_AGENTS;
  int percentage = 100;

  Torus2D map(n, m);
  Configuration<Torus2D> config(map);
  config.init();
  
  for (int i = 0; i < num_agents; i++)
  {
    Agent agent(i, *map.get_vertex(rand() % n, rand() % m));
    config.add_agent(agent);
  }

  config.parallel_setup();

  int iter = 0;
  std::cout<<"Starting simulation"<<std::endl;
  auto t1 = std::chrono::high_resolution_clock::now();

  while (!config.is_finished()) {
    config.transition();
    iter++;

    if (MAX_ITERS > 0 && iter >= MAX_ITERS)
      break;
    if (verbose && iter % 100 == 0) {
      std::cout << "Iteration: " << iter <<std::endl;
    }
  }

  auto t2 = std::chrono::high_resolution_clock::now();
  config.print_config();

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