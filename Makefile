json: demo_json.c tb_json.c
	gcc demo_json.c tb_json.c -o json -Wall -std=c99

jwrite: demo_jwrite.c tb_jwrite.c
	gcc demo_jwrite.c tb_jwrite.c -o jwrite -Wall -std=c99

file: demo_file.c tb_file.c
	gcc demo_file.c tb_file.c -o file -Wall -std=c99