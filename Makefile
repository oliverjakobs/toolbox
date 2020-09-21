# json
json: demo/demo_json.c src/tb_json.c
	gcc demo/demo_json.c src/tb_json.c -o json -Wall -std=c99

# jwrite
jwrite: demo/demo_jwrite.c src/tb_jwrite.c
	gcc demo/demo_jwrite.c src/tb_jwrite.c -o jwrite -Wall -std=c99

# file
file: demo/demo_file.c src/tb_file.c
	gcc demo/demo_file.c src/tb_file.c -o file -Wall -g -std=c99

# array
array: demo/demo_array.c src/tb_array.c
	gcc demo/demo_array.c src/tb_array.c -o array -Wall -std=c99

# list
list: demo/demo_list.c src/tb_list.c
	gcc demo/demo_list.c src/tb_list.c -o list -Wall -std=c99

# hashmap

# map

# bits
bits: demo/demo_bits.c src/tb_bits.c
	gcc demo/demo_bits.c src/tb_bits.c -o bits -Wall -std=c99
