import numpy as np

def get_sift_vectors(filename):
	raw = np.fromfile(filename, dtype=np.float32)
	raw = np.reshape(raw, (len(raw) // 129, 129))
	return np.delete(raw, 0, 1)


base = "sift/"
dataset = get_sift_vectors(base + "sift_base.fvecs")
learn = get_sift_vectors(base + "sift_learn.fvecs")
query = get_sift_vectors(base + "sift_query.fvecs")

np.save(base + "data", dataset)
np.save(base + "learn", learn)
np.save(base + "query", query)

import os
if not os.path.isfile(base + "query_gt.npy"):
  query_gt = []
  for i in range(len(query)):
    result = np.linalg.norm(dataset - query[i], axis=1)
    query_gt.append(min(result))
  np.save(base + "query_gt", np.array(query_gt))

if not os.path.isfile(base + "learn_gt.npy"):
  learn_gt = []
  for i in range(len(learn)):
    result = np.linalg.norm(dataset - learn[i], axis=1)
    learn_gt.append(min(result))
    if i % 1000 == 0:
      print(i)
  np.save(base + "learn_gt", np.array(learn_gt))


