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
  int percentage = 100;

  Torus3D map(n, m, o);
  Configuration<Torus3D> config(map);
  config.init();
  
  for (int i = 0; i < num_agents; i++)
  {
    Agent agent(i, *map.get_vertex(rand() % n, rand() % m, rand() % o));
    config.add_agent(agent);
  }

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