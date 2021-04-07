# import the necessary packages
import os

# Set up number of visible gpus
os.environ['CUDA_VISIBLE_DEVICES'] = '2'

# specify the shape of the inputs for our network
IMG_SHAPE = (28, 28, 1)

# specify the batch size and number of epochs
BATCH_SIZE = 64
EPOCHS = 100

# define the path to the base output directory
BASE_OUTPUT = "output"

# use the base output path to derive the path to the serialized
# model along with training history plot
MODEL_PATH = os.path.sep.join([BASE_OUTPUT, "siamese_model"])
PLOT_PATH = os.path.sep.join([BASE_OUTPUT, "plot.png"])