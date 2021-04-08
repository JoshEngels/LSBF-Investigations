# import the necessary packages
from tensorflow.keras.models import Sequential
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input
from tensorflow.keras.layers import Conv2D
from tensorflow.keras.layers import Dense
from tensorflow.keras.layers import Dropout
from tensorflow.keras.layers import Flatten
from tensorflow.keras.layers import GlobalAveragePooling2D
from tensorflow.keras.layers import MaxPooling2D

def build_siamese_model(num_input_dimensions):

  model = Sequential()
  model.add(Input(shape=(num_input_dimensions,)))
  # model.add(Dense(4 * num_input_dimensions, activation="relu"))
  return model

  # By adding 3 dense layers I got 99% validation accuracy but I couldn't really repeat it, still did get 90% with one
  # model = Sequential()
  # model.add(Input(shape=inputShape))
  # model.add(Flatten())
  # model.add(Dense(embeddingDim, activation="relu"))
  # return model

	# # define the first set of CONV => RELU => POOL => DROPOUT layers
	# x = Conv2D(64, (2, 2), padding="same", activation="relu")(inputs)
	# x = MaxPooling2D(pool_size=(2, 2))(x)
	# x = Dropout(0.3)(x)

	# # second set of CONV => RELU => POOL => DROPOUT layers
	# x = Conv2D(64, (2, 2), padding="same", activation="relu")(x)
	# x = MaxPooling2D(pool_size=2)(x)
	# x = Dropout(0.3)(x)

	# # prepare the final outputs
	# pooledOutput = GlobalAveragePooling2D()(x)
	# outputs = Dense(embeddingDim)(pooledOutput)

	# # build the model
	# model = Model(inputs, outputs)

	# return the model to the calling function
	# return model