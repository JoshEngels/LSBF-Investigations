import sys

input_file = sys.argv[1]
print_type = sys.argv[2] # Query or similarity

with open(input_file) as f:
	while True:

		line = f.readline().strip()
		if (line == ""):
			break

		last_space_pos = -1
		while (line[last_space_pos] != " "):
			last_space_pos -= 1
		if (print_type == "query"):
			print(line[:last_space_pos])
		else:
			print(line[last_space_pos + 1:])
