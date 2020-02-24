json: demo_json.c
	gcc demo_json.c -o json -Wall -std=c99

json_write: demo_json_write.c
	gcc demo_json_write.c -o json_write -Wall -std=c99

tiny_math: demo_math.c tiny_math.c
	gcc demo_math.c tiny_math.c -o math -Wall -std=c99

file: demo_file.c
	gcc demo_file.c -o file -Wall -std=c99