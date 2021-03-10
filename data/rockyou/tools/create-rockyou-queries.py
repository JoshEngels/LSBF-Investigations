import sys
import io
import random

rockyou_data = sys.argv[1]

def get_sim(data_kmers, query_kmers):
	best_sim = 0
	for item in data_kmers:
		
		current_index = 0
		num_same = 0
		for kmer in item:
			while(current_index < len(query_kmers) and query_kmers[current_index] < kmer):
					current_index += 1
			if current_index == len(query_kmers):
				break
			if query_kmers[current_index] == kmer:
				num_same += 1
				current_index += 1

		test_sim = float(num_same) / (len(query_kmers) + len(item) - num_same)
		best_sim = max(best_sim, test_sim)

	return best_sim

def get_kmers(string, kmer_size):
	kmers = set()
	for i in range(0, len(string) - kmer_size + 1):
		kmers.add(string[i: i + kmer_size])
	kmer_list = list(kmers)
	kmer_list.sort()
	return kmer_list

take_from = "abcdefghiklmnopqrstuvwxyz1234567890!@#$%^&*()"
def create_query(original, num_changes):
	result = original
	for i in range(num_changes):
		random_pos = int(random.random() * len(result))
		random_char = take_from[int(random.random() * len(take_from))]
		decider = random.random()
		if (decider < 0.5 and len(result) > 3):
			# Do a delete
			result = result[:random_pos] + result[random_pos + 1:]
		elif (decider < 0.7):
			result += random_char
		else:
			result = result[:random_pos] + random_char + result[random_pos + 1:]
	return result

data_kmers = []
data = []
kmer_size = 3
with io.open(rockyou_data, mode="r", encoding="latin-1") as f:
	while True:
		line = f.readline()
		if (line == ""):
			break
		# if (len(data_kmers) % 10000 == 0):
		# 	print(len(data_kmers))
		data_kmers.append(get_kmers(line.strip(), kmer_size))
		data.append(line.strip())

for i in range(10000):
	query = create_query(data[int(random.random() * len(data))], i % 8 + 1)
	sim = get_sim(data_kmers[1:], get_kmers(query, kmer_size))
	print (query, f"{sim:.2f}", flush=True)
	
