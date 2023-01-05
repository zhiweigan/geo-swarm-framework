#include <configuration.h>
#include <geo_utils.h>
#include <parlay/parallel.h>
#include <parlay/primitives.h>
#include <set>

#define CRESET "\033[0m"
#define CBLACK "\033[30m"   /* Black */
#define CRED "\033[31m"     /* Red */
#define CGREEN "\033[32m"   /* Green */
#define CYELLOW "\033[33m"  /* Yellow */
#define CBLUE "\033[34m"    /* Blue */
#define CMAGENTA "\033[35m" /* Magenta */
#define CCYAN "\033[36m"    /* Cyan */
#define CWHITE "\033[37m"   /* White */

void Configuration::update_agents()
{
  // parlay::parallel_for(0, agent_ids.size(), [&](int i)
  // {
  //   auto &transition = agent_transitions[i];
  //   agents[agent_ids[i]].state = transition.astate;
  //   Position pos = get_coords_from_movement(agents[agent_ids[i]].loc->loc, transition.dir);
  //   agents[agent_ids[i]].loc = map.get_vertex(pos.x, pos.y);
  //   transition.accepted = true;
  // });
}

Configuration::~Configuration()
{ }

void Configuration::custom_setup()
{ }

void Configuration::update_locations()
{ }

bool Configuration::is_finished()
{
  // int total_no_color = parlay::reduce(parlay::delayed_map(agents, [&](Agent agent)
  // { 
  //   return agent.state.color == SampleColor::NONE; 
  // }));
  // return total_no_color == 0;
  return false;
}

void Configuration::print_config(int time, int flags)
{  
  std::cout << "Percent of Agents in None/Blue/Red: " << std::endl;
  double percent_red = 0;
  double percent_blue = 0;
  double percent_none = 0;

  std::vector<std::vector<int>> output;
  for (int i = 0; i < n; i++)
  {
    std::vector<int> s;
    for (int j = 0; j < m; j++)
    {
      s.push_back(0);
    }
    output.push_back(s);
  }

  for (auto agent : agents)
  {
    Position loc = agent.loc->loc;
    if (agent.state.color == SampleColor::RED)
    {
      output[loc.x][loc.y] += 1;
      percent_red += 1;
    }
    else if (agent.state.color == SampleColor::BLUE)
    {
      percent_blue += 1;
      output[loc.x][loc.y] -= 1;
    }
    else 
    {
      percent_none += 1;
    }
    output[loc.x][loc.y] = std::max(-9, std::min(9, output[loc.x][loc.y]));
  }

  if (flags & PRINT_OUT)
  {
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < m; j++)
      {
        if (output[i][j] == 0)
          std::cout << output[i][j];
        if (output[i][j] > 0)
          std::cout << CRED << output[i][j] << CRESET;
        if (output[i][j] < 0)
          std::cout << CBLUE << std::abs(output[i][j]) << CRESET;
      }
      std::cout << std::endl;
    }
  }

  std::cout << percent_none / agents.size() << std::endl;
  std::cout << percent_blue / agents.size() << std::endl;
  std::cout << percent_red / agents.size() <<std::endl;
}