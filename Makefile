json: main.c
	gcc main.c -o json -Wall -std=c99

json_write: write_main.c json_write.c
	gcc write_main.c json_write.c -o json_write -Wall -std=c99