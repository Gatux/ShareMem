ALL: default 

CC           = gcc
CLINKER      = $(CC)
OPTFLAGS     = -O0


SHELL = /bin/sh

CFLAGS  =   -DREENTRANT
CCFLAGS = $(CFLAGS)
LIBS =  -lpthread

EXECS = exemple exemple2

default: $(EXECS)

exemple2: exemple2.o dsm.o common.o
	$(CLINKER) $(OPTFLAGS) -o exemple2 exemple2.o dsm.o common.o $(LIBS)

exemple: exemple.o dsm.o common.o
	$(CLINKER) $(OPTFLAGS) -o exemple exemple.o dsm.o common.o $(LIBS)

clean:
	@-/bin/rm -f *.o *~ PI* $(EXECS) *.out core  
.c:
	$(CC) $(CFLAGS) -o $* $< $(LIBS)
.c.o:
	$(CC) $(CFLAGS) -c $<
.o:
	${CLINKER} $(OPTFLAGS) -o $* $*.o $(LIBS)
