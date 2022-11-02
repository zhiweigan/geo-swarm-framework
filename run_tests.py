import subprocess
import os

cores_to_try = [1,2,4,8,16]
tries = 1
results = {}
for cores in cores_to_try:
  total = 0
  for t in range(tries):
    print("On cores:", cores, "try:", t)
    os.putenv('PARLAY_NUM_THREADS', str(cores))
    subprocess.run(['make', 'clean'], stdout=subprocess.PIPE)
    subprocess.run(['make', '-j12'], stdout=subprocess.PIPE)
    result = subprocess.run(['build/final_program'], stdout=subprocess.PIPE)
    time = result.stdout.split(b' ')[-2]
    total += int(time)
  total /= tries
  results[cores] = total

print(results)

