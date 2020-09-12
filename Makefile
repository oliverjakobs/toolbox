# json
json: demo/demo_json.c tb_json.c
	gcc demo/demo_json.c tb_json.c -o json -Wall -std=c99

# jwrite
jwrite: demo/demo_jwrite.c tb_jwrite.c
	gcc demo/demo_jwrite.c tb_jwrite.c -o jwrite -Wall -std=c99

# file
file: demo/demo_file.c tb_file.c
	gcc demo/demo_file.c tb_file.c -o file -Wall -std=c99

# array

# list
list: demo/demo_list.c tb_list.c
	gcc demo/demo_list.c tb_list.c -o list -Wall -std=c99

# hashmap

# map

# bits
bits: demo/demo_bits.c tb_bits.c
	gcc demo/demo_bits.c tb_bits.c -o bits -Wall -std=c99
