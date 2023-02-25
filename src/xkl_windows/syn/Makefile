# $Log: Makefile,v $
# Revision 1.5  2000/04/20 05:33:16  krishna
# Changed install to copy executables instead of moving them.
#
# Revision 1.4  1998/07/16 23:03:24  krishna
# XKLDIR passed to syn.c telling it where to look for library files.
#
# Revision 1.3  1998/06/09 00:45:14  krishna
# Added RCS header.
#

######################################################################
#
# Makefile for syn.  No need for user to edit in here.
#
#######################################################################

include ../Makefile.vars

PROGS = syn

LIB_FILES = default60.con

INCLUDES = -I../common

XLIBS_DIR = -L/usr/X11/lib -L/usr/lib/X11 $(XTRA_LIBS_DIR)
LIBS = $(XLIBS_DIR)

COMMON_OBJS = ../common/wavio.o

OBJS = synmain.o syn.o syngrid.o synscrip.o synparwav.o

all: $(PROGS)

install:
	chmod go+rx $(PROGS) $(SCRIPTS)
	cp -f $(PROGS) $(SCRIPTS) $(BINDIR)
	cp -f $(LIB_FILES) $(LIBDIR)

clean:	
	rm -f *.o core *~ $(PROGS) $(OBJS)

syn: $(OBJS) $(COMMON_OBJS)
	$(CC) -o $@ $(OBJS) $(COMMON_OBJS) $(LIBS) -lm

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $*.c

syn.o: syn.c ../common/wavio.h
	$(CC) -DXKLDIR=\"${LIBDIR}\" ${INCLUDES} -c -o syn.o syn.c

synmain.o: synmain.c
syngrid.o: syngrid.c
synscrip.o: synscrip.c
synparwav.o: synparwav.c
