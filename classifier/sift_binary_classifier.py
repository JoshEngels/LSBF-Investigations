
import numpy as np
import os
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
from sklearn.utils import class_weight


os.environ['CUDA_VISIBLE_DEVICES'] = '2'
cutoff = 80
# num_train_points = 5000

query = np.load("../data/sift/query.npy")
query_gt = np.load("../data/sift/query_gt.npy")
train = np.load("../data/sift/learn.npy")
train_gt = np.load("../data/sift/learn_gt.npy")

# train = combined[:num_train_points]
# query = combined[num_train_points:]

train_gt = train_gt < cutoff
query_gt = query_gt < cutoff

print("NONZERO", np.count_nonzero(query_gt))

# # Calculate the weights for each class so that we can balance the data
weights = class_weight.compute_class_weight('balanced',
                                            classes=[False, True],
                                            y=train_gt)

model = Sequential()
model.add(Dense(4 * 128, input_dim=128, activation='relu'))
model.add(Dense(128, input_dim=4 * 128, activation='relu'))
model.add(Dense(1, activation='sigmoid'))
model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy', 'AUC'])
model.summary()

model.fit(train, train_gt, validation_data=(query, query_gt), epochs=50, batch_size=64, class_weight={i:weights[i] for i in range(len(weights))})
# model.fit(train, train_gt, validation_data=(query, query_gt), epochs=10, batch_size=32)



