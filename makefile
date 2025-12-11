compile:
	@echo " Compiling conv_main.c ...";
	gcc -I ./include/ ./program/conv_main.c ./src/*.c  -o ./build/conv_main;


run: compile
	@echo " Running conv_main ...";
	./build/conv_main

