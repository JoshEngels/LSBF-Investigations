from math import *

# See https://stackoverflow.com/questions/809362/how-to-calculate-cumulative-normal-distribution
def phi(x):
    #'Cumulative distribution function for the standard normal distribution'
    return (1.0 + erf(x / sqrt(2.0))) / 2.0

def calculate_sim(distance, r):
	return 1 - 2 * phi(-r / distance) - 2 / (sqrt(2 * pi) * r / distance) * (1 - exp(-r*r / 2 / (distance * distance)))

# with open("../data/sift/query-distances"):
  
r = 300
print(calculate_sim(1, r))
print(calculate_sim(50, r))
print(calculate_sim(100, r))
print(calculate_sim(200, r))
print(calculate_sim(300, r))