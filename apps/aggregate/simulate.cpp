#include <chrono>

#include <map.h>
#include <math.h>
#include <geo_utils.h>
#include <configuration.h>
#include <parlay/primitives.h>

double get_heat(int x, int y, int n, int m)
{
  double heat = exp(-pow((double)x-(double)n/2,2)/100 - pow((double)y-(double)m/2,2)/100) * 2;
  if (heat < 0.05) return 0;
  return heat;
}

// run a small test case
int
main()
{
  // constants
  bool verbose = false;
  int n = HEIGHT, m = WIDTH;
  int num_agents = NUM_AGENTS;

  Torus2D map(n, m);
  Configuration<Torus2D> config(map);
  config.init();
  
  for (int i = 0; i < num_agents; i++)
  {
    Agent agent(i, *map.get_vertex(rand() % (n-2) + 1, rand() % (m-2) + 1));
    config.add_agent(agent);
  }

  for (int16_t i = 0; i < n; i++)
  {
    for(int16_t j = 0; j < m; j++)
    {
      AggregateLocationState state(Position{i,j});
      state.heat = get_heat(i,j,n,m);
      state.isWall = false;
      config.map.get_vertex(i,j)->state = state;
    }
  }

  for(int i = 0; i < n; i++) config.map.get_vertex(i, 0)->state.isWall = true;
  for(int i = 0; i < n; i++) config.map.get_vertex(i, m-1)->state.isWall = true;
  for(int j = 0; j < m; j++) config.map.get_vertex(0, j)->state.isWall = true;
  for(int j = 0; j < m; j++) config.map.get_vertex(n-1, j)->state.isWall = true;
  

  config.parallel_setup();

  int iter = 0;
  std::cout<<"Starting simulation"<<std::endl;
  // config.print_config();

  while (!config.is_finished()) {
    config.transition();
    iter++;

    if (MAX_ITERS > 0 && iter >= MAX_ITERS)
      break;
    if (iter % INT == 0) {
      std::cout << "Iteration: " << iter <<std::endl;
      config.print_config();
    }
    if (DEBUG)
    {
      config.print_config();
    }
    
  }

  // config.print_config();

  return 0;
}