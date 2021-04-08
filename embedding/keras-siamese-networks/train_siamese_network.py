# USAGE
# python train_siamese_network.py
import numpy as np

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

# training parameters
num_positive_train = 25000
num_negative_train = 25000
num_positive_test = 2000
num_negative_test = 2000

# define problem
cutoff = 250

# load SIFT dataset
print("[INFO] loading SIFT dataset...")
sift_hf = h5py.File(config.DATA_PATH, "r")
dataset = sift_hf['train'][:]

# prepare the positive and negative pairs
print("[INFO] preparing positive and negative pairs...")
positive_pairs, negative_pairs = utils.get_stored_pairs(cutoff)
print(len(positive_pairs), num_positive_test, num_positive_train)
assert(len(positive_pairs) > num_positive_test + num_positive_train)
assert(len(negative_pairs) > num_negative_test + num_negative_train)

(pairTrain, labelTrain) = utils.get_pairs(dataset, positive_pairs[0:num_positive_train], negative_pairs[0:num_negative_train])
(pairTest, labelTest) = utils.get_pairs(dataset, positive_pairs[num_positive_train:num_positive_train + num_positive_test], negative_pairs[num_negative_train:num_negative_train + num_negative_test])

# configure the siamese network
print("[INFO] building siamese network...")
imgA = Input(shape=config.INPUT_SHAPE)
imgB = Input(shape=config.INPUT_SHAPE)
featureExtractor = build_siamese_model(config.INPUT_SHAPE[0])
featsA = featureExtractor(imgA)
featsB = featureExtractor(imgB)

# finally, construct the siamese network
similarity = Lambda(utils.euclidean_hash_prob)([featsA, featsB])
# TODO: Change the outputs to be another lambda that does collision probability
# outputs = Dense(1, activation="sigmoid")(distance)
model = Model(inputs=[imgA, imgB], outputs=similarity)
# exit()

# compile the model
print("[INFO] compiling model...")
model.compile(loss="binary_crossentropy", optimizer="adam",
	metrics=["accuracy"])

print(model.summary())

# train the model
print("[INFO] training model...")
history = model.fit(
	[pairTrain[:, 0], pairTrain[:, 1]], labelTrain[:],
	validation_data=([pairTest[:, 0], pairTest[:, 1]], labelTest[:]),
	batch_size=config.BATCH_SIZE, 
	epochs=1)

# print(positive_pairs[0])
# print(negative_pairs[0])
# print(model.predict([pairTrain[:2, 0], pairTrain[:2, 1]]))
# print(model.predict([pairTrain[:2, 0], pairTrain[:2, 1]]).shape)
exit()

# serialize the model to disk
print("[INFO] saving siamese model...")
model.save(config.MODEL_PATH)

# plot the training history
print("[INFO] plotting training history...")
utils.plot_training(history, config.PLOT_PATH)