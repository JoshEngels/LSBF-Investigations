# What we do here is basically pick good hyperparameters by hand on the training
# set (for simplicity sake), then test on the testing set and put it all on a
# nice plot. We choose filter size = 8 * dataset size so chance collisons 
# only happen with p = 1 / 64. We choose r such that the average number of 
# collisions on the training set is 1 / 2 (so p = 1 / 64 is not a big deal),
# and we choose a concatenation number (with r) that is "large enough" (the 
# AUC on the training data set does not sizably increase if we increase it).
# I tried to do this all with code automatically earlier but this if fine for now.

import lsbf
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

# Set up for plot
matplotlib.use("agg")
titlefontsize = 22
axisfontsize = 18
labelfontsize = 12
ls = "--"


# Filter size
for num_filters in [20, 40, 80, 160]:
	one_filter_size = 8000000  # 16 bits per filter per point

	# Half the points are for training
	num_train_points = 5000
	key = 42

	# Read in dataset
	import h5py
	hf = h5py.File('../data/sift/sift-128-euclidean.hdf5', 'r')
	dataset = hf['train'][:]
	all_ground = hf['distances'][:,0]
	data_dim = dataset.shape[1]
	num_data_points = dataset.shape[0]

	# Set up train and query datasets
	train = hf['test'][:num_train_points]
	query = hf['test'][num_train_points:]

	num_buckets = 10
	min_dist = min(all_ground)
	max_dist = max(all_ground)
	interval_size = (max_dist - min_dist) / (num_buckets + 2)
	cutoffs = [min_dist + i * interval_size for i in range(1, num_buckets + 1)]


	# These all get about 50% collisions on the training dataset, just did binary search by hand
	# lead_to_about_half = [(38, 4), (120, 6), (210, 8), (300, 10), (380, 12), (450, 14), (530, 16), (610, 18), (680, 20)]
	# We'll just use (530, 16), it works decently well
	lead_to_about_half = [(530, 16)]

	aucs = []
	for cutoff in cutoffs:
		
		# Set up ground truth
		ground = np.where(all_ground < cutoff, True, False)
		train_gt = ground[:num_train_points]
		query_gt = ground[num_train_points:]	


		best_auc_train = 0
		best_auc_test = 0
		for (r, concatenation_num) in lead_to_about_half:
			l_filter = lsbf.LSBF_Euclidean(cutoff, data_dim, one_filter_size, num_filters, num_data_points, num_train_points, key)
			l_filter.setup(dataset, r, concatenation_num)
			if (l_filter.getAUC(train, train_gt) > best_auc_train):
				best_auc_test = l_filter.getAUC(query, query_gt) 
		aucs.append(best_auc_test)

	print(aucs)
	plt.plot(cutoffs, aucs, linestyle=ls, label=f"{num_filters * one_filter_size / 1000000} Bits/Point")

plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize='xx-small')
plt.ylabel("AUC", fontsize=axisfontsize)
plt.xlabel(f"Distance Threshold", fontsize=axisfontsize)
plt.title(f"AUC of LSBF By Distance Threshold", fontsize=titlefontsize)
plt.savefig(f"AUC-sift.png", bbox_inches='tight')

plt.close()

