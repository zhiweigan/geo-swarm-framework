import subprocess
import os
import json

lines_to_map = {
  1: 'checking',
  2: 'sorting',
  3: 'generating',
  4: 'updating',
  5: 'iterations',
  6: 'total'
}

cores_to_try = [1,2,4,8,16]
tries = 5
results = {}
os.chdir("../../build")
for cores in cores_to_try:
  checking = 0
  sorting = 0
  generating = 0
  updating  = 0
  iterations = 0
  total = 0

  ret_map = {
    'checking': 0,
    'sorting': 0,
    'generating': 0,
    'updating': 0,
    'iterations': 0,
    'total': 0 
  }

  
  for t in range(tries):
    print("On cores:", cores, "try:", t)
    os.putenv('PARLAY_NUM_THREADS', str(cores))
    subprocess.run(['make', 'clean'], stdout=subprocess.PIPE)
    subprocess.run(['make', '-j12'], stdout=subprocess.PIPE)
    result = subprocess.run(['./alloc'], stdout=subprocess.PIPE)
    lines = result.stdout.split(b'\n')

    for line_num in range(1, len(lines)-1):
      line = lines[line_num]
      print(line.decode("utf-8"))
      ret_map[lines_to_map[line_num]] += float(line.split(b' ')[-2])

  for key in ret_map:
    ret_map[key] /= tries
  results[cores] = ret_map

print(json.dumps(results, indent=4))

