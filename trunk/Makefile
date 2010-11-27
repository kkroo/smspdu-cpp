#
# Makefile
#
OBJ =	src/main.o		\
		src/pdu.o		
#
DEP =	include/pdu.h	
CC 		= g++
CCFLAGS = -Wall -g3 -Iinclude
#
%.o:		%.cpp $(DEP)
	$(CC) -c -o $@ $< $(CCFLAGS)

smspdu: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	
clean:
	rm -rf $(OBJ) smspdu

