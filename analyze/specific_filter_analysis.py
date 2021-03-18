# Call with e.g.
#  python3 specific_filter_analysis.py ../output/rockyou-results ../data/rockyou/query-sims 15 22 2 3 sim
#  python3 specific_filter_analysis.py ../output/sift-results ../data/sift/query-distances 50 22 1 20 dist
#  python3 specific_filter_analysis.py ../output/sift-results ../data/sift/query-distances 50 22 1 20 dist-to-sim


import sys 
from math import *

# See https://stackoverflow.com/questions/809362/how-to-calculate-cumulative-normal-distribution
def phi(x):
    #'Cumulative distribution function for the standard normal distribution'
    return (1.0 + erf(x / sqrt(2.0))) / 2.0

def calculate_sim(distance, r):
	return 1 - 2 * phi(-r / distance) - 2 / (sqrt(2 * pi) * r / distance) * (1 - exp(-r*r / 2 / (distance * distance)))

file_name = sys.argv[1]
query_file = sys.argv[2]
num_hashes = int(sys.argv[3])
hash_bits = int(sys.argv[4])
hash_range = 2**hash_bits
multiplicity = sys.argv[5]
r_value = int(sys.argv[6])
compare_type = sys.argv[7]

if compare_type not in ["sim", "dist", "dist-to-sim"]:
	print("Invalid compare type")
	exit(0)

ground = []
with open(query_file, "r") as f:
	while True:
		line = f.readline().strip()
		if line == "":
			break
		ground.append(float(line))

if (compare_type == "sim" or compare_type == "dist-to-sim"):
	r_cutoffs = [0.05 * i for i in range(7, 19)]
else:
	r_cutoffs = [20 * i for i in range(2, 17)]
false_negative_counts = [[0 for i in range(len(r_cutoffs))] for t in range(num_hashes + 1)]
false_positive_counts = [[0 for i in range(len(r_cutoffs))] for t in range(num_hashes + 1)]
positive_counts = [0 for i in range(len(r_cutoffs))]
negative_counts = [0 for i in range(len(r_cutoffs))]
query_index = 0
with open(file_name) as f:
	while True:
		
		line = f.readline().strip()
		if line == "":
			print("Requested stats not found, ending")
			exit(1)
		if line.startswith("Threshold") and f"{num_hashes} {hash_range} {multiplicity} {r_value}" in line:
			
			line = f.readline().strip()
			while not line.startswith("Threshold"):
			
				# Parse threshold score
				threshold = int(line)
				if (compare_type == "sim" or compare_type == "dist"):
					query_gt = ground[query_index]
				else:
					query_gt = calculate_sim(ground[query_index], r_value)
				query_index += 1
			
				# Update fpr and fnr tracking
				for cutoff_index, cutoff in enumerate(r_cutoffs):
					
					if compare_type == "sim" or compare_type == "dist-to-sim":
						positive = cutoff <= query_gt
					else:
						positive = query_gt <= cutoff

					if positive:
							positive_counts[cutoff_index] += 1 
							for t in range(threshold + 1, num_hashes + 1): # Update false negative
								false_negative_counts[t][cutoff_index] += 1
					else:
						negative_counts[cutoff_index] += 1 # Negative
						for t in range(0, threshold + 1): # Update false positive
							false_positive_counts[t][cutoff_index] += 1

				line = f.readline()

			break

# Calculate rates
print(positive_counts)
print(negative_counts)
print(false_negative_counts)
print(false_positive_counts)
false_negative_rates = [[count / positive_counts[i] for i, count in enumerate(arr)] for arr in false_negative_counts]
false_positive_rates = [[count / negative_counts[i] for i, count in enumerate(arr)] for arr in false_positive_counts]

# Do plotting
import matplotlib
import matplotlib.pyplot as plt
matplotlib.use("agg")
titlefontsize = 22
axisfontsize = 18
labelfontsize = 12
ls = "--"

for threshold, results in enumerate(false_positive_rates):
	if threshold % 2 == 0:
		plt.plot(r_cutoffs, results, linestyle=ls, label=f"Threshold: {threshold}")
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize='xx-small')
plt.ylabel("FPR", fontsize=axisfontsize)
plt.xlabel(f"r-Threshold ({compare_type})", fontsize=axisfontsize)
plt.title(f"FPR of LSBF: {num_hashes} hashes,\n{multiplicity} concatenations, {hash_bits} hash bits", fontsize=titlefontsize)
plt.savefig(f"FPR-{num_hashes}-{multiplicity}-{hash_bits}.png", bbox_inches='tight')

plt.close()

for threshold, results in enumerate(false_negative_rates):
	if threshold % 2 == 0:
		plt.plot(r_cutoffs, results, linestyle=ls, label=f"Threshold: {threshold}")
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize='xx-small')
plt.ylabel("FNR", fontsize=axisfontsize)
plt.xlabel(f"r-Threshold ({compare_type})", fontsize=axisfontsize)
plt.title(f"FNR of LSBF: {num_hashes} hashes,\n{multiplicity} concatenations, {hash_bits} hash bits", fontsize=titlefontsize)
plt.savefig(f"FNR-{num_hashes}-{multiplicity}-{hash_bits}.png", bbox_inches='tight')
