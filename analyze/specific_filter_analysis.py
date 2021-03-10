import sys 

# Call with e.g. python3 specific_filter_analysis.py ../output/rockyou-trial-results ../data/rockyou/rockyou-queries-gt 15 22 2
file_name = sys.argv[1]
query_file = sys.argv[2]
num_hashes = int(sys.argv[3])
hash_bits = int(sys.argv[4])
hash_range = 2**hash_bits
multiplicity = sys.argv[5]

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
false_negative_counts = [[0 for i in range(len(r_cutoffs))] for t in range(num_hashes + 1)]
false_positive_counts = [[0 for i in range(len(r_cutoffs))] for t in range(num_hashes + 1)]
positive_counts = [0 for i in range(len(r_cutoffs))]
negative_counts = [0 for i in range(len(r_cutoffs))]
with open(file_name) as f:
  while True:
    
    line = f.readline().strip()
    if line == "":
      print("Requested stats not found, ending")
      break
    if line.startswith("Threshold") and line.endswith(f"13 1 {num_hashes} {hash_range} {multiplicity}"):
      
      line = f.readline()
      while not line.startswith("Threshold"):
      
        # Parse password and threshold score
        last_space = -2
        while line[last_space] != " ":
          last_space -= 1
        password = line[:last_space]
        threshold = int(line[last_space + 1:])
        query_sim = sim_map[password]
      
        # Update fpr and fnr tracking
        for cutoff_index, cutoff in enumerate(r_cutoffs):
          
          if cutoff <= query_sim: # Positive
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
  if threshold % (num_hashes // 7) == 0:
	  plt.plot(r_cutoffs, results, linestyle=ls, label=f"Threshold: {threshold}")
plt.legend(loc='upper right')
plt.ylabel("FPR", fontsize=axisfontsize)
plt.xlabel("Similarity Threshold", fontsize=axisfontsize)
plt.title(f"FPR of LSBF: {num_hashes} hashes,\n{multiplicity} concatenations, {hash_bits} hash bits", fontsize=titlefontsize)
plt.savefig(f"FPR-{num_hashes}-{multiplicity}-{hash_bits}.png".title(), bbox_inches='tight')

plt.close()

for threshold, results in enumerate(false_negative_rates):
  if threshold % (num_hashes // 7) == 0:
	  plt.plot(r_cutoffs, results, linestyle=ls, label=f"Threshold: {threshold}")
plt.legend(loc='upper right')
plt.ylabel("FNR", fontsize=axisfontsize)
plt.xlabel("Similarity Threshold", fontsize=axisfontsize)
plt.title(f"FNR of LSBF: {num_hashes} hashes,\n{multiplicity} concatenations, {hash_bits} hash bits", fontsize=titlefontsize)
plt.savefig(f"FNR-{num_hashes}-{multiplicity}-{hash_bits}.png", bbox_inches='tight')
