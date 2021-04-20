from model_details import config
from model_details import utils
from model_details.utils import collision_prob_contrastive_loss
from tensorflow.keras.models import load_model
from tensorflow.keras.models import Sequential
import h5py
import numpy as np
import sys
import random
from math import *


def phi(x):
    #'Cumulative distribution function for the standard normal distribution'
    return (1.0 + erf(x / sqrt(2.0))) / 2.0

def calculate_sim(distance, r):
	return 1 - 2 * phi(-r / distance) - 2 / (sqrt(2 * pi) * r / distance) * (1 - exp(-r*r / 2 / (distance * distance)))

model = load_model(config.MODEL_PATH, custom_objects={'collision_prob_contrastive_loss': collision_prob_contrastive_loss})

model.summary()

# Remove last lambda layer
new_model = Sequential()
for layer in model.layers[:-1]: # go through until last layer
  new_model.add(layer)

new_model.summary()

# ------------------------ Run tests with embedded data ---------------------- #

# Read in dataset
sift_hf = h5py.File(config.DATA_PATH, "r")
dataset = sift_hf['train'][:]
all_ground = sift_hf['distances'][:,0]
data_dim = dataset.shape[1]
num_data_points = dataset.shape[0]
num_train_points = 5000

# Set up train and query datasets
train = sift_hf['test'][:num_train_points]
query = sift_hf['test'][num_train_points:]

cutoff = 250

# Set up ground truth
ground = np.where(all_ground < cutoff, True, False)
train_gt = ground[:num_train_points]
query_gt = ground[num_train_points:]	

import lsbf

dataset_embeddings = new_model.predict(dataset)
query_embeddings = new_model.predict(query)
train_embeddings = new_model.predict(train)
original_aucs = []
embedded_aucs = []
rep_array =  [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]
for num_filters in rep_array:
# for num_filters in rep_array:

  data_dim = 128
  l_filter = lsbf.LSBF_Euclidean(cutoff, data_dim, config.ONE_FILTER_SIZE, num_filters, num_data_points, num_train_points, config.KEY)
  l_filter.setup(dataset_embeddings, config.HASH_R, config.CONCATENATION_NUM)
  embedded_aucs.append(l_filter.getAUC(train_embeddings, train_gt))

  data_dim = 128
  l_filter = lsbf.LSBF_Euclidean(cutoff, data_dim, config.ONE_FILTER_SIZE, num_filters, num_data_points, num_train_points, config.KEY)
  l_filter.setup(dataset, 530, 16)
  original_aucs.append(l_filter.getAUC(train, train_gt))


import matplotlib as plt
import matplotlib
import matplotlib.pyplot as plt

# Set up for plot
matplotlib.use("agg")
titlefontsize = 22
axisfontsize = 18
labelfontsize = 12
ls = "--"

# Plot
plt.plot(range(1, len(rep_array) + 1), original_aucs, linestyle=ls, label=f"Original AUCS")
plt.plot(range(1, len(rep_array) + 1), embedded_aucs, linestyle=ls, label=f"Embedded AUCS")
plt.legend()
plt.ylabel("AUC", fontsize=axisfontsize)
plt.xlabel(f"Log2(# Filter Reps), each filter = {int(config.ONE_FILTER_SIZE / 1000000)} bits/poins", fontsize=axisfontsize)
plt.title(f"AUC By Number of Reps, Cutoff = 250", fontsize=titlefontsize)
plt.savefig(f"Embedding-VS-Original-{config.CURRENT_MODEL}-{int(config.ONE_FILTER_SIZE / 1000000)}-best", bbox_inches='tight')

