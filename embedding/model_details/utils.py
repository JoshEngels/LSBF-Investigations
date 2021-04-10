# import the necessary packages
import matplotlib.pyplot as plt
from model_details import config
import math
import tensorflow as tf
import numpy as np

def get_stored_pairs(cutoff):
	positive_pairs = []
	negative_pairs = []
	with open(config.STORED_PAIRS_PATH) as f:
		while True:
			line = f.readline()
			if not line:
				break 
			
			if len(line.split()) < 4:
				continue
			first_index, _, distance, second_index = line.split()
			if first_index != second_index:
				if (float(distance) < cutoff):
					positive_pairs.append((int(first_index), int(second_index)))
				else:
					negative_pairs.append((int(first_index), int(second_index)))

	return positive_pairs, negative_pairs

def get_pairs(dataset, positive_pairs, negative_pairs):
	assert(len(positive_pairs) == len(negative_pairs)) # Because I'm lazy

	# pairs and labels
	data_pairs = []
	labels = []

	for i in range(len(positive_pairs)):
		labels.append(1)
		data_pairs.append([dataset[positive_pairs[i][0]], dataset[positive_pairs[i][1]]])
		labels.append(0)
		data_pairs.append([dataset[negative_pairs[i][0]], dataset[negative_pairs[i][1]]])
	
	# return a 2-tuple of our image pairs and labels
	return (np.array(data_pairs), np.array(labels))

def euclidean_distance(vectors):
	import tensorflow.keras.backend as K

	# unpack the vectors into separate lists
	(featsA, featsB) = vectors

	# compute the sum of squared distances between the vectors
	sumSquared = K.sum(K.square(featsA - featsB), axis=1,
		keepdims=True)

	# return the euclidean distance between the vectors
	return K.sqrt(K.maximum(sumSquared, K.epsilon()))

def euclidean_hash_prob(vectors):
	import tensorflow.keras.backend as K

	r = 367.5 # This makes the 0.50 cutoff be exactly at the 250, which makes the accuracy be almost 1.0

	dist = euclidean_distance(vectors)
	
	cdf = 0.5 + 0.5 * tf.math.erf((-r / dist) / (2 ** 0.5))
	first_term = 1 - 2 * cdf

	second_term = 2 / ((math.pi * 2) ** 0.5) / (r / dist) * (1 - K.exp(- r * r / dist / dist / 2))

	return first_term - second_term

def collision_prob_contrastive_loss(y, collision_probs, margin=1):
	import tensorflow.keras.backend as K

	# explicitly cast the true class label data type to the predicted
	# class label data type (otherwise we run the risk of having two
	# separate data types, causing TensorFlow to error out)
	y = tf.cast(y, collision_probs.dtype)
	
	# calculate the contrastive loss between the true labels and
	# the predicted labels
	positive_loss = 1 - collision_probs 
	negative_loss = K.maximum(margin + collision_probs - 1, 0)
	loss = K.mean(y * (positive_loss ** 2) + (1 - y) * (negative_loss ** 2))
	return loss



def plot_training(H, plotPath):
	# construct a plot that plots and saves the training history
	plt.style.use("ggplot")
	plt.figure()
	plt.plot(H.history["loss"], label="train_loss")
	plt.plot(H.history["val_loss"], label="val_loss")
	plt.plot(H.history["accuracy"], label="train_acc")
	plt.plot(H.history["val_accuracy"], label="val_acc")
	plt.title("Training Loss and Accuracy")
	plt.xlabel("Epoch #")
	plt.ylabel("Loss/Accuracy")
	plt.legend(loc="lower left")
	plt.savefig(plotPath)