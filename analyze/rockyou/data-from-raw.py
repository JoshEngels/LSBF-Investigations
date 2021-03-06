import sys

input_file = sys.argv[1]

with open(input_file) as f:
	while True:
		line = f.readline()
		if (line == ""):
			break
		print(line.split()[0])