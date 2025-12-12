compile:
	@echo " Compiling conv_main.c ...";
	gcc -I ./include/ ./program/conv_main.c ./src/*.c  -o ./build/conv_main;


run: compile
	@echo " Running conv_main with input 01234 ...";
	./build/conv_main 01234

run_alt: compile
	@echo " Running conv_main with input 56789 ...";
	./build/conv_main 56789

flush: compile
	@echo " Flushing conv_main with input 01234 ...";
	./build/conv_main 01234 FLUSH

flush_alt: compile
	@echo " Flushing conv_main with input 56789 ...";
	./build/conv_main 56789 FLUSH