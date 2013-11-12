#CC		= gcc
#PCFLAGS      	= -Iinc -Isrc/headers -lm -g
#SRCS        	= $(wildcard src/*.c)
#MAINOBJS    	=  hello/hello.o  @ this is one level up of the main folder




default:
	@echo "============================================================"
	@echo "----------------HTTP - Bluetooth Server---------------------"
	@echo "Compile the source code ----------------------> make compile"
	@echo "Run the system -------------------------------> make run"
	@echo "============================================================"
		
	
#
# General C compilation dependencies
#
#.c.o:
#$(CC) -c $< -o $@ $(PCFLAGS)


compile:
	@echo "Compiling source code against -lbluetooth and -lpthread ..."
	@echo "============================================================"		
	gcc http.c server.c -o sim -lbluetooth -lpthread
	@echo "============================================================"
	@echo "Done!"
	@echo "You may run the system now."
	@echo "============================================================"

# run Linux
run:			
	./sim
