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

# large:
large_raw = {
  "1": {
      "checking": -115.89420000000003,
      "Sorting Agents": 609833.0,
      "Generating Transitions": 5003444.0,
      "Updating Agents and Locations": 433660.8,
      "Iterations": 154833.6,
      "Total Time": 6131931.0
  },
  "2": {
      "checking": -108.01079999999999,
      "Sorting Agents": 358807.0,
      "Generating Transitions": 2422330.0,
      "Updating Agents and Locations": 242437.2,
      "Iterations": 155853.0,
      "Total Time": 3074197.2
  },
  "4": {
      "checking": -19.751459999999998,
      "Sorting Agents": 228899.4,
      "Generating Transitions": 1197230.0,
      "Updating Agents and Locations": 132988.0,
      "Iterations": 154175.6,
      "Total Time": 1582344.6
  },
  "8": {
      "checking": 1073.324,
      "Sorting Agents": 183028.8,
      "Generating Transitions": 639308.2,
      "Updating Agents and Locations": 90040.37999999999,
      "Iterations": 154641.0,
      "Total Time": 927252.2
  },
  "16": {
      "checking": 1193.0704,
      "Sorting Agents": 165671.2,
      "Generating Transitions": 341761.2,
      "Updating Agents and Locations": 67954.94,
      "Iterations": 156645.0,
      "Total Time": 585232.8
  }
}

categories = large_raw["1"].keys()
legend = {}
for cat in categories:
  legend[cat] = [(int(key), large_raw["1"][cat] / elem[cat]) for key, elem in large_raw.items()]

# plt.plot(none_x, none_y, color = 'r')
# plt.plot(parlay_rand_x, parlay_rand_y, color = 'g')
# plt.plot(by_agent_x, by_agent_y, color='r')



fig = plt.figure()
ax = plt.subplot(111)
for key, values in legend.items():
  if key == 'checking': continue
  if key == 'Iterations': continue
  values_x, values_y = zip(*values)
  ax.plot(values_x, values_y, label=key)
box = ax.get_position()
ax.set_position([box.x0, box.y0, box.width * 0.6, box.height])
ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
fig.suptitle('Parallelism Plot for Large Data', fontsize=20)
plt.xlabel('Cores', fontsize=18)
plt.ylabel('Parallelism', fontsize=16)


plt.show()
