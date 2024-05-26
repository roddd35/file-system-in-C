CC=gcc
CFLAGS= -Wall -g 

################################

OBJS=ep3.o

MAIN=ep3.c -lreadline

################################

.PHONY: clean

all: ep

ep2.o: ep3.h

ep: ${OBJS}
		${CC} ${CFLAGS} -o ep3 ${MAIN}
		@rm -rf *.o *.dSYM .vscode .DS_Stores

clean:
	@rm -rf newsh ep3 *.o .vscode .DS_Store *.dSYM .fuse_hidden0000009200000002