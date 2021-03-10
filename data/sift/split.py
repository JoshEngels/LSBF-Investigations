import h5py
import sys

f = h5py.File('sift-128-euclidean.hdf5', 'r')

# Print query distances
with open("query-distances", "w+") as qd:
  distances = f['distances']
  firsts = [distances[i][0] for i in range(len(distances))]
  firsts.sort()
  for d in distances:
    qd.write(str(d[0]) + "\n")
 
  import matplotlib.pyplot as plt 
  plt.hist(firsts, density=False, bins=30)  
  plt.show()

# Print queries
with open("queries", "w+") as q:
  test = f['test']
  for point in test:
    row = " ".join([str(p) for p in point]) + "\n"
    q.write(row)


# Print data
with open("data", "w+") as d: 
  train = f['train']
  for point in train:
    row = " ".join([str(p) for p in point]) + "\n"
    d.write(row)

