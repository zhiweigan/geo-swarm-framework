import subprocess
import os
import json

percentage = 0
tries = 100
results = []
os.chdir("../../")
os.putenv('PARLAY_NUM_THREADS', '8')
for i in range(tries):
  print("try:", i)
  subprocess.run(['make', 'clean'], stdout=subprocess.PIPE)
  subprocess.run(['make', '-j12'], stdout=subprocess.PIPE)
  result = subprocess.run(['build/final_program'], stdout=subprocess.PIPE)
  lines = result.stdout.split(b'\n')
  
  percentage += float(lines[-2])
  results.append(float(lines[-2]))

print(results)
print(percentage / tries)
