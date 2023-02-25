# $Log: Makefile,v $
# Revision 1.6  1998/06/06 05:06:24  krishna
# Added RCS header.
#
#
######################################################################
#
# Makefile for common.  No need for user to edit in here.
#
######################################################################

include ../Makefile.vars

COMMON_OBJS = wavaudio.o wavio.o spectrum.o getargs.o wavdata.o

all: ${COMMON_OBJS}

install:
	@echo "No installation done in this directory."

clean:	
	rm -f *.o core *~ ${COMMON_OBJS}

waveaudio.o: waveaudio.c waveaudio.h
waveio.o: waveio.c waveio.h
spectrum.o: spectrum.c spectrum.h
getargs.o: getargs.c getargs.h
wavdata.o: wavdata.c wavdata.h
