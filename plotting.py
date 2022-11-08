import matplotlib.pyplot as plt
import numpy as np

# no speedups:
none = {1: 7711.6, 2: 6368.6, 4: 5365.6, 8: 2990.0, 16: 1370.4}
none_list = sorted(none.items())
none_x, none_y = zip(*none_list)

# parlay_rand:
parlay_rand = {1: 10478.0, 2: 6127.2, 4: 3432.0, 8: 1886.8, 16: 1675.0}
parlay_rand_list = sorted(parlay_rand.items())
parlay_rand_x, parlay_rand_y = zip(*parlay_rand_list)

# parlay_sort_by_agent:
by_agent_raw = {1: 3083.2, 2: 2658.0, 4: 1941.6, 8: 1613.4, 16: 1545.0}
by_agent_list = [(key, by_agent_raw[1] / elem) for key, elem in by_agent_raw.items()]
# by_agent_list = sorted(by_agent.items())
by_agent_x, by_agent_y = zip(*by_agent_list)

# plt.plot(none_x, none_y, color = 'r')
# plt.plot(parlay_rand_x, parlay_rand_y, color = 'g')
plt.plot(by_agent_x, by_agent_y, color='r')
plt.show()
