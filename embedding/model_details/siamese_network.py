# import the necessary packages
from tensorflow.keras.models import Sequential
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input
from tensorflow.keras.layers import Dense

def build_siamese_model(num_input_dimensions):

  model = Sequential()
  model.add(Input(shape=(num_input_dimensions,)))
  model.add(Dense(4 * num_input_dimensions, activation="relu"))
  model.add(Dense(num_input_dimensions, activation="relu"))
  return model
