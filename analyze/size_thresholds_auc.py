# Example call from within folder: 
# python3 size_thresholds_auc.py <results> <query similarities> <dataset title> <num_points> <comparison type> <r (for sim and dist-to-sim)>
# python3 size_thresholds_auc.py ../output/rockyou-results ../data/rockyou/query-sims rockyou 1000000 sim 10
# python3 size_thresholds_auc.py ../output/sift-results ../data/sift/query-distances sift 1000000 dist
# python3 size_thresholds_auc.py ../output/sift-results ../data/sift/query-distances sift 1000000 dist-to-sim 300

import sys 
from math import *

# See https://stackoverflow.com/questions/809362/how-to-calculate-cumulative-normal-distribution
def phi(x):
    #'Cumulative distribution function for the standard normal distribution'
    return (1.0 + erf(x / sqrt(2.0))) / 2.0

def calculate_sim(distance, r):
	return 1 - 2 * phi(-r / distance) - 2 / (sqrt(2 * pi) * r / distance) * (1 - exp(-r*r / 2 / (distance * distance)))
	
output_file = sys.argv[1]
query_file = sys.argv[2]
dataset_name = sys.argv[3]
num_points = int(sys.argv[4])
compare_type = sys.argv[5]
if compare_type in ["sim", "dist-to-sim"]:
	r_value = int(sys.argv[6])

if compare_type not in ["sim", "dist", "dist-to-sim"]:
	print("Invalid compare type")
	exit(0)

# Assume straight line between points
def calculate_auc(tprs, fprs):
	auc = 0
	tprs = tprs[::-1]
	fprs = fprs[::-1]
	last_x = 0
	last_y = 0
	for x, y in zip(fprs, tprs):
		auc += (x - last_x) * (last_y + y) / 2
		last_y = y
		last_x = x
	auc += (1 - x) * (1 + y) / 2
	# print(tprs, fprs, auc)
	return auc


ground = []
with open(query_file, "r") as f:
	while True:
		line = f.readline().strip()
		if line == "":
			break
		ground.append(float(line))

num_buckets = 10
if (compare_type == "sim"):
	min_sim = min(ground)
	max_sim = max(ground)
	interval_size = (max_sim - min_sim) / (num_buckets + 2)
	r_cutoffs = [min_sim + i * interval_size for i in range(1, num_buckets + 1)]
elif (compare_type == "dist-to-sim"):
	ground_sims = [calculate_sim(d, r_value) for d in ground]
	min_sim = min(ground_sims)
	max_sim = max(ground_sims)
	interval_size = (max_sim - min_sim) / (num_buckets + 2)
	r_cutoffs = [min_sim + i * interval_size for i in range(1, num_buckets + 1)]
else:
	min_dist = min(ground)
	max_dist = max(ground)
	interval_size = (max_dist - min_dist) / (num_buckets + 2)
	r_cutoffs = [min_dist + i * interval_size for i in range(1, num_buckets + 1)]
	
best_aucs = [0 for i in range(len(r_cutoffs))]
best_stats = [[] for i in range(len(r_cutoffs))]
best_accuracies = [0 for i in range(len(r_cutoffs))]

# Track aucs for size thresholds
size_thresholds = [2 ** i for i in range(-2, 12)]
best_aucs_sized = [[0 for i in range(len(r_cutoffs))] for j in range(len(size_thresholds))]
best_stats_sized = [[[] for i in range(len(r_cutoffs))] for j in range(len(size_thresholds))]


with open(output_file, "r") as output:
	line = output.readline().strip()
	while True:
		if line == "":
			break
		
		# Here, because of file format current line starts with Threshold
		print(line)
		words = line.split()
		statistics = [int(i) for i in words[words.index("statistics") + 1:]]
		bits_per_item = statistics[2] * statistics[3] / num_points
		# print(bits_per_item)
		# Need an entry for every threshold, cutoff pair to generate auc 
		true_positive_counts = [[0 for _ in range(statistics[2])] for _ in range(len(r_cutoffs))]
		false_positive_counts = [[0 for _ in range(statistics[2])] for _ in range(len(r_cutoffs))]
		total_positive = [0 for _ in range(len(r_cutoffs))]
		total_negative = [0 for _ in range(len(r_cutoffs))]
		query_index = 0

		while True:
			line = output.readline().strip()
			if line.startswith("Threshold") or line == "":
				break
			
			# Parse threshold score
			threshold = int(line)
			if (compare_type == "sim" or compare_type == "dist"):
				query_gt = ground[query_index]
			else:
				query_gt = calculate_sim(ground[query_index], int(statistics[5]))
			query_index += 1

			# Fill in entries in counts arrays
			for i, cutoff in enumerate(r_cutoffs):
				
				if compare_type == "sim" or compare_type == "dist-to-sim":
					positive = cutoff <= query_gt
				else:
					positive = query_gt <= cutoff


				if positive:
					total_positive[i] += 1
				else:
					total_negative[i] += 1

				for t in range(statistics[2]):
						if t <= threshold and positive: # True positive
							true_positive_counts[i][t] += 1
						if t <= threshold and not positive: # False positive
							false_positive_counts[i][t] += 1


		# Calculate rates
		# print(total_positive)
		# print(total_negative)
		true_positive_rates = [[count / total_positive[i] for count in arr] for i, arr in enumerate(true_positive_counts)]
		false_positive_rates = [[count / total_negative[i] for count in arr] for i, arr in enumerate(false_positive_counts)]

		# Calculate and update aucs
		for i, (tprs, fprs) in enumerate(zip(true_positive_rates, false_positive_rates)):
			auc = calculate_auc(tprs, fprs)
			
			# Update aucs
			if auc > best_aucs[i]:
				best_aucs[i] = auc
				best_stats[i] = statistics
			for index, size_threshold in enumerate(size_thresholds):
				if bits_per_item > size_threshold:
					continue 
				if best_aucs_sized[index][i] < auc:
					best_aucs_sized[index][i] = auc
					best_stats_sized[index][i] = statistics

		# Calculate and update accuracy
		for i in range(len(r_cutoffs)):
			for t in range(statistics[2]):
				best_accuracies[i] = max(best_accuracies[i], (true_positive_counts[i][t] + total_negative[i] - false_positive_counts[i][t]) / (total_negative[i] + total_positive[i]))
		

# print(best_aucs)
# print(best_stats)
# print(best_accuracies)
# print(best_aucs_sized)
# print(best_stats)
# print(best_stats_sized)


titlefontsize = 22
axisfontsize = 18
labelfontsize = 12
ls = "--"

import matplotlib
import matplotlib.pyplot as plt
matplotlib.use("agg")
for size_results, size_threshold in zip(best_aucs_sized, size_thresholds):
	if (size_results[0] != 0):
		plt.plot(r_cutoffs, size_results, linestyle=ls, label=f"Max {size_threshold} bits")
plt.legend(loc='lower right')
plt.ylabel("AUC", fontsize=axisfontsize)
plt.xlabel(f"r-Threshold ({compare_type})", fontsize=axisfontsize)

if (compare_type in ["sim", "dist-to-sim"]):
	plt.title(f"Best AUC of LSBF on {dataset_name}-{num_points}-{r_value}", fontsize=titlefontsize)
else:
	plt.title(f"Best AUC of LSBF on {dataset_name}-{num_points}", fontsize=titlefontsize)
if (compare_type in ["sim", "dist-to-sim"]):
	plt.savefig(f"AUC-{dataset_name}-{num_points}-{r_value}.png", bbox_inches='tight')
else:
	plt.savefig(f"AUC-{dataset_name}-{num_points}.png", bbox_inches='tight')


