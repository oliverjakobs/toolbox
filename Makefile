# algorithm
algorithm: demo/demo_algorithm.c src/tb_algorithm.c
	gcc demo/demo_algorithm.c src/tb_algorithm.c -o algorithm -Wall -std=c99

# array
array: demo/demo_array.c src/tb_array.c
	gcc demo/demo_array.c src/tb_array.c -o array -Wall -std=c99

# file
file: demo/demo_file.c src/tb_file.c
	gcc demo/demo_file.c src/tb_file.c -o file -Wall -std=c99

# hashmap
hashmap: demo/demo_hashmap.c src/tb_hashmap.c
	gcc demo/demo_hashmap.c src/tb_hashmap.c -o hashmap -Wall -std=c99

# ini
ini: demo/demo_ini.c src/tb_ini.c
	gcc demo/demo_ini.c src/tb_ini.c -o ini -Wall -std=c99

# mem
mem: demo/demo_mem.c src/tb_mem.c
	gcc demo/demo_mem.c src/tb_mem.c -o mem -Wall -std=c99

# str
str: demo/demo_str.c src/tb_str.c
	gcc demo/demo_str.c src/tb_str.c -o str -Wall -std=c99