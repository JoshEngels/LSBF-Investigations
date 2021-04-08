import h5py
import numpy as np
hf = h5py.File('/scratch0/jae4/LSBF-Investigations/data/sift/sift-128-euclidean.hdf5', 'r')
dataset = hf['train'][:]
all_ground = hf['distances'][:,0]
data_dim = dataset.shape[1]
num_data_points = dataset.shape[0]

for i in range(0, 1000000, 10):
  result = np.linalg.norm(dataset - dataset[i], axis=1)
  sortedResult = np.sort(result)
  sortedArgs = np.argsort(result)

  jump = 1
  current = 0
  nextStop = 100
  while (current < 1000000):
    print(i, current, sortedResult[current], sortedArgs[current])
    if (current == nextStop):
      nextStop *= 100
      jump *= 100
    current += jump
  