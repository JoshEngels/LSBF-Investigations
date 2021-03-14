# Example call from within folder: python3 size_thresholds_auc.py ../output/rockyou-trial-results ../data/rockyou/rockyou-queries-gt rockyou 1000000

import sys 

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


query_file = sys.argv[2]
sim_map = {}
with open(query_file, "r") as f:
	while True:
		line = f.readline().strip()
		if line == "":
			break
		password = line[:-5]
		sim = float(line[-4:])
		sim_map[password] = sim


r_cutoffs = [0.05 * i for i in range(2, 18)]
best_aucs = [0 for i in range(len(r_cutoffs))]
best_stats = [[] for i in range(len(r_cutoffs))]
best_accuracies = [0 for i in range(len(r_cutoffs))]

# Track aucs for size thresholds
size_thresholds = [2 ** i for i in range(-2, 12)]
best_aucs_sized = [[0 for i in range(len(r_cutoffs))] for j in range(len(size_thresholds))]

output_name = sys.argv[1]
with open(output_name, "r") as output_file:
	line = output_file.readline().strip()
	while True:
		if line == "":
			break
		
		# Here, because of file format current line starts with Threshold
		statistics = [int(i) for i in line.split()[-5:]]
		bits_per_item = statistics[2] * statistics[3] / int(sys.argv[4])
		# print(bits_per_item)
		# Need an entry for every threshold, cutoff pair to generate auc 
		true_positive_counts = [[0 for _ in range(statistics[2])] for _ in range(len(r_cutoffs))]
		false_positive_counts = [[0 for _ in range(statistics[2])] for _ in range(len(r_cutoffs))]
		total_positive = [0 for _ in range(len(r_cutoffs))]
		total_negative = [0 for _ in range(len(r_cutoffs))]

		while True:
			line = output_file.readline().strip()
			if line.startswith("Threshold") or line == "":
				break
			
			# Parse password and threshold score
			last_space = -2
			while line[last_space] != " ":
				last_space -= 1
			password = line[:last_space]
			threshold = int(line[last_space + 1:])
			query_sim = sim_map[password]

			# Fill in entries in counts arrays
			for i, cutoff in enumerate(r_cutoffs):
				
				if cutoff <= query_sim: # Positive
					total_positive[i] += 1
				if cutoff > query_sim: # Negative
					total_negative[i] += 1 
				
				for t in range(statistics[2]):
						if t <= threshold and cutoff <= query_sim: # True positive
							true_positive_counts[i][t] += 1
						if t <= threshold and cutoff > query_sim: # False positive
							false_positive_counts[i][t] += 1

		# Calculate rates
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
				best_aucs_sized[index][i] = max(best_aucs_sized[index][i], auc)

		# Calculate and update accuracy
		for i in range(len(r_cutoffs)):
			for t in range(statistics[2]):
				best_accuracies[i] = max(best_accuracies[i], (true_positive_counts[i][t] + total_negative[i] - false_positive_counts[i][t]) / (total_negative[i] + total_positive[i]))
		

print(best_aucs)
print(best_stats)
print(best_accuracies)
print(best_aucs_sized)


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
plt.xlabel("Similarity Threshold", fontsize=axisfontsize)
plt.title(f"Best AUC of LSBF on {sys.argv[3]}-{sys.argv[4]}", fontsize=titlefontsize)
plt.savefig(f"AUC-{sys.argv[3]}-{sys.argv[4]}.png", bbox_inches='tight')

