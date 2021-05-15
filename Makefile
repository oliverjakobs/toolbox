# array
array: demo/demo_array.c src/tb_array.c
	gcc demo/demo_array.c src/tb_array.c -o array -Wall -std=c99

# bits
bits: demo/demo_bits.c src/tb_bits.c
	gcc demo/demo_bits.c src/tb_bits.c -o bits -Wall -std=c99

# config
config: demo/demo_config.c src/tb_config.c
	gcc demo/demo_config.c src/tb_config.c -o config -Wall -std=c99

# file

# hashmap

# ini

# json
json: demo/demo_json.c src/tb_json.c
	gcc demo/demo_json.c src/tb_json.c -o json -Wall -std=c99

# jwrite
jwrite: demo/demo_jwrite.c src/tb_jwrite.c
	gcc demo/demo_jwrite.c src/tb_jwrite.c -o jwrite -Wall -std=c99

# mem

# str

# toolbox