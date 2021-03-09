import sys

input_file = sys.argv[1]

with open(input_file) as f:
	while True:

		line = f.readline()
		if (line == ""):
			break

		last_space_pos = -1
		while (line[last_space_pos] != " "):
			last_space_pos -= 1
		print(line[:last_space_pos])
