#include <chrono>

#include <map.h>
#include <math.h>
#include <geo_utils.h>
#include <configuration.h>
#include <atomic>
#include <parlay/primitives.h>

double get_heat(int x, int y, int n, int m)
{
  double heat = exp(-pow((double)x-(double)n/2,2)/100 - pow((double)y-(double)m/2,2)/100) * 2;
  // if (heat < 0.05) return 0;
  return heat;
}

std::atomic<int> population[HEIGHT][WIDTH] = {};

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

  for (int16_t i = 0; i < n; i++)
  {
    for(int16_t j = 0; j < m; j++)
    {
      AggregateLocationState state(Position{i,j});
      state.heat = get_heat(i,j,n,m);
      state.isWall = false;
      state.population = &(population[i][j]);
      population[i][j] = 0;
      config.map.get_vertex(i,j)->state = state;
    }
  }
  
  for (int i = 0; i < num_agents; i++)
  {
    int x, y;

    do  {
      x = rand() % (n-2) + 1;
      y = rand() % (m-2) + 1;
    } while((population[x][y] != 0));

    Agent agent(i, *map.get_vertex(x,y));
    population[x][y]++;
    
    config.add_agent(agent);
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
    if (iter % INT == 0 && !DEBUG) {
      std::cout << "Iteration: " << iter <<std::endl;
      config.print_config(iter, 1);
    }
    if (DEBUG)
    {
      config.print_config(iter, 0);
    }
    
  }

  // config.print_config();

  return 0;
}