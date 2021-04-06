from math import *

# See https://stackoverflow.com/questions/809362/how-to-calculate-cumulative-normal-distribution
def phi(x):
    #'Cumulative distribution function for the standard normal distribution'
    return (1.0 + erf(x / sqrt(2.0))) / 2.0

def calculate_sim(distance, r):
	return 1 - 2 * phi(-r / distance) - 2 / (sqrt(2 * pi) * r / distance) * (1 - exp(-r*r / 2 / (distance * distance)))

  
# r = 400.0
# print(calculate_sim(1, r))
# print(calculate_sim(50, r))
# print(calculate_sim(100, r))
# print(calculate_sim(200.0, r)**10)
# print(calculate_sim(300, r))

# print(calculate_sim(3**(0.5), 5)**4)
# print(calculate_sim(17**(0.5), 5)**4)
# print(calculate_sim(22**(0.5), 5)**4)

# print(calculate_sim(200, 22)**3)
# print(calculate_sim(200, 74)**4)
# print(calculate_sim(200, 112)**5)
# print(calculate_sim(200, 112)**6)
# print(calculate_sim(100, 400)**10)


# WHY IS IT SO GOOD FOR THIS? IS IT UNION BOUND? And just that near points  
# are more likely to be kinda near to lots of points, have a higher sum of similarity measure,
# I forget what that's called. And this is just approximating that.
# Worry is that actual good hash collision probability is small even when less than
# threshold, so to fix this will have to greatly change weight in a way that is not
# unbiased (i.e. maybe weight change is in terms of number of points, who knows)

def get_specific_prob(dist):
  return calculate_sim(dist, 400)**10

import h5py
hf = h5py.File('data/sift/sift-128-euclidean.hdf5', 'r')
all_ground = hf['distances']

import matplotlib.pyplot as plt
import numpy as np
points = [(all_ground[i, 0], min(sum([get_specific_prob(all_ground[i, j]) for j in range(len(all_ground[i]))]), 1)) for i in range(1000)]
def scatter_hist(x, y, ax, ax_histx):
    # no labels
    ax_histx.tick_params(axis="x", labelbottom=False)
    # ax_histy.tick_params(axis="y", labelleft=False)

    # the scatter plot:
    ax.scatter(x, y)

    # now determine nice limits by hand:
    # xymax = max(np.max(np.abs(x)), np.max(np.abs(y)))
    # lim = (int(xymax/binwidth) + 1) * binwidth

    binwidth = 10
    lowerlim = np.min(x)
    upperlim = np.max(x)

    bins = np.arange(lowerlim, upperlim, binwidth)
    ax_histx.hist(x, bins=bins)
    ax_histx.set_ylabel('Num points')
    # ax_histy.hist(y, bins=bins, orientation='horizontal')

# definitions for the axes
left, width = 0.1, 0.65
bottom, height = 0.1, 0.65
spacing = 0.005

rect_scatter = [left, bottom, width, height]
rect_histx = [left, bottom + height + spacing, width, 0.2]
# rect_histy = [left + width + spacing, bottom, 0.2, height]

# start with a square Figure
fig = plt.figure(figsize=(8, 8))

ax = fig.add_axes(rect_scatter)
ax.set_xlabel('Queries')
ax.set_ylabel('Sum(Collision prob of 100 nns), r = 400, concats = 10')
ax_histx = fig.add_axes(rect_histx, sharex=ax)
# ax_histy = fig.add_axes(rect_histy, sharey=ax)

# use the previously defined function
scatter_hist([x for x, y in points], [y for x, y in points], ax, ax_histx)

# plt.scatter([x for x, y in points], [y for x, y in points])
plt.savefig("temp.png")