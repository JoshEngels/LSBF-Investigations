# import the necessary packages
import os

# Set up number of visible gpus
os.environ['CUDA_VISIBLE_DEVICES'] = '1'

# specify the shape of the inputs for our network
INPUT_SHAPE = (128,)

# specify the batch size and number of epochs
BATCH_SIZE = 64
EPOCHS = 20

# define the path to the base output directory
BASE_OUTPUT = "output"

# use the base output path to derive the path to the serialized
# model along with training history plot
MODEL_PATH = os.path.sep.join([BASE_OUTPUT, "siamese_model_squared_loss"])
PLOT_PATH = os.path.sep.join([BASE_OUTPUT, "plot.png"])

DATA_PATH = "/scratch0/jae4/LSBF-Investigations/data/sift/sift-128-euclidean.hdf5"
STORED_PAIRS_PATH = "/scratch0/jae4/LSBF-Investigations/embedding/sift-pairs/pairs"