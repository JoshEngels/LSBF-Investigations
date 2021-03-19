import lsbf
import numpy as np

cutoff = 200
data_dim = 3
one_filter_size = 1000000
num_filters = 100
num_data_points = 2
num_train_points = 1
key = 42

a = lsbf.LSBF_Euclidean(cutoff, data_dim, one_filter_size, num_filters, num_data_points, num_train_points, key)
data = np.array([[1.0,2.0, 3.1], [1.3,2.0, 4]]) 
train = np.array([[1.1, 2.1, 3.2]])
ground = np.array([True])
a.setupAndTrain(data, train, ground)
