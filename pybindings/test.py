import lsbf
import numpy as np

cutoff = 200
one_filter_size = 10000000
num_filters = 100
num_train_points = 5000
key = 42

# Read in dataset
import h5py
hf = h5py.File('../data/sift/sift-128-euclidean.hdf5', 'r')
dataset = hf['train'][:]
ground = hf['distances'][:,0]
ground = np.where(ground < cutoff, True, False)
data_dim = dataset.shape[1]
num_data_points = dataset.shape[0]

train_gt = ground[:num_train_points]
train = hf['test'][:num_train_points]

query_gt = ground[num_train_points:]
query = hf['test'][num_train_points:]


a = lsbf.LSBF_Euclidean(cutoff, data_dim, one_filter_size, num_filters, num_data_points, num_train_points, key)
a.setupAndTrain(dataset, train, train_gt)
