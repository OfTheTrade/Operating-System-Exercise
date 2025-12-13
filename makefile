# Run to compile conv_main in build/
compile:
	@echo " Compiling conv_main.c ...";
	gcc -I ./include/ ./program/conv_main.c ./src/*.c  -o ./build/conv_main;

# Run to execute conv_main with "01234" as the conversation id input
run: compile
	@echo " Running conv_main with input 01234 ...";
	./build/conv_main 01234

# Run to execute conv_main with "56789" as the conversation id input
run_alt: compile
	@echo " Running conv_main with input 56789 ...";
	./build/conv_main 56789

# Run to flush the shared memory and semaphore of conv_main (with "01234" as the conversation id input)
flush: compile
	@echo " Flushing conv_main with input 01234 ...";
	./build/conv_main 01234 FLUSH

# Run to flush the shared memory and semaphore of conv_main (with "56789" as the conversation id input)
flush_alt: compile
	@echo " Flushing conv_main with input 56789 ...";
	./build/conv_main 56789 FLUSH