# USAGE
# python train_siamese_network.py

# import the necessary packages
from pyimagesearch.siamese_network import build_siamese_model
from pyimagesearch import config
from pyimagesearch import utils
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Dense
from tensorflow.keras.layers import Input
from tensorflow.keras.layers import Lambda
from tensorflow.keras.datasets import mnist
import h5py
import random

# NOTE: We get loss of 0.4, validation loss of 0.4 WITHOUT any learning
# using the contrastive loss on the collision probabilities (and accuracy of 1 lol).
# NOTE: We get loss of 0.23 or so with learning with a single layer on 1000000 points
# and 300000 points after 10 epochs (0.25 after two epochs), not bad
# TODO: Add more "hard" pairs
# TODO: Think about loss function, how we weight positive and negative examples
# TODO: Could square or do something else to penalize collision probs
# TODO: Try different margins
# TODO: Problem: what if we just get better at hard pairs and worse at easy pairs, how can we make a loss function to fix this?

# training parameters
num_positive_train = 1500000
num_negative_train = 1500000
num_positive_test = 500000
num_negative_test = 500000

# define problem
cutoff = 250

# load SIFT dataset
print("[INFO] Loading SIFT dataset...")
sift_hf = h5py.File(config.DATA_PATH, "r")
dataset = sift_hf['train'][:]

# prepare the positive and negative pairs
print("[INFO] Preparing positive and negative pairs...")
positive_pairs, negative_pairs = utils.get_stored_pairs(cutoff)
print(f"[INFO] Getting {num_positive_train} positive train pairs and {num_positive_test} positive test pairs from {len(positive_pairs)} total positive pairs.")
assert(len(positive_pairs) > num_positive_test + num_positive_train)
assert(len(negative_pairs) > num_negative_test + num_negative_train)

# Randomize order of lists
random.shuffle(positive_pairs)
random.shuffle(negative_pairs)

(pairTrain, labelTrain) = utils.get_pairs(dataset, positive_pairs[0:num_positive_train], negative_pairs[0:num_negative_train])
(pairTest, labelTest) = utils.get_pairs(dataset, positive_pairs[num_positive_train:num_positive_train + num_positive_test], negative_pairs[num_negative_train:num_negative_train + num_negative_test])

# configure the siamese network
print("[INFO] Building siamese network...")
imgA = Input(shape=config.INPUT_SHAPE)
imgB = Input(shape=config.INPUT_SHAPE)
featureExtractor = build_siamese_model(config.INPUT_SHAPE[0])
featsA = featureExtractor(imgA)
featsB = featureExtractor(imgB)

# finally, construct the siamese network
similarity = Lambda(utils.euclidean_hash_prob)([featsA, featsB])
model = Model(inputs=[imgA, imgB], outputs=similarity)

# compile the model
print("[INFO] compiling model...")
model.compile(loss=utils.collision_prob_contrastive_loss, optimizer="adam",
	metrics=["accuracy"])

print(model.summary())

# train the model
print("[INFO] training model...")
history = model.fit(
	[pairTrain[:, 0], pairTrain[:, 1]], labelTrain[:],
	validation_data=([pairTest[:, 0], pairTest[:, 1]], labelTest[:]),
	batch_size=config.BATCH_SIZE, 
	epochs=config.EPOCHS)


# serialize the model to disk
print("[INFO] Saving siamese model...")
model.save(config.MODEL_PATH)

# plot the training history
print("[INFO] Plotting training history...")
utils.plot_training(history, config.PLOT_PATH)