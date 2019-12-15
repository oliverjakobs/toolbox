json: main.c
	gcc main.c -o json -Wall -std=c99

json_write: main_write.c json_write.c
	gcc main_write.c json_write.c -o json_write -Wall -std=c99