# $Log: Makefile,v $
# Revision 1.6  2000/04/20 05:33:51  krishna
# Changed install to copy executables instead of moving them.
#
# Revision 1.5  1998/06/12 05:07:46  krishna
# Modified so that only lsp.c is compiled with XKLDIR definition.
#
# Revision 1.4  1998/06/12 04:58:21  krishna
# Added LIBDIR definition so lspecto looks in right directory for
# LIB_FILES.
#
# Revision 1.3  1998/06/06 21:05:16  krishna
# Added RCS header.
#

#######################################################################
#
# Makefile for lspecto.  No need for user to edit in here.
#
#######################################################################

include ../Makefile.vars

PROGS = lspecto

LIB_FILES = lspecto.con

INCLUDES = -I../common

XLIBS_DIR = -L/usr/X11/lib -L/usr/lib/X11 $(XTRA_LIBS_DIR)
LIBS = $(XLIBS_DIR)

COMMON_OBJS = ../common/wavio.o ../common/spectrum.o

OBJS = lsp.o lspspec.o lspformant.o lspgetf0.o lspfbank.o \
	lspdft.o lspgrid.o lspscrip.o


all:: $(PROGS)

install:
	chmod go+rx $(PROGS) $(SCRIPTS)
	cp -f $(PROGS) $(SCRIPTS) $(BINDIR)
	cp -f $(LIB_FILES) $(LIBDIR)

clean:	
	rm -f *.o core *~ $(PROGS) $(OBJS)

lspecto: $(OBJS) $(COMMON_OBJS)
	$(CC) -o $@ $(OBJS) $(COMMON_OBJS) $(LIBS) -lm

lsp.o: lsp.c ../common/wavio.h ../common/wavio.o
	$(CC) -DXKLDIR=\"${LIBDIR}\" ${INCLUDES} -c -o lsp.o lsp.c

lspspec.o: lspspec.c ../common/spectrum.h ../common/spectrum.o
lspformant.o: lspformant.c
lspgetf0.o: lspgetf0.c
lspfbank.o: lspfbank.c
lspdft.o: lspdft.c
lspgrid.o: lspgrid.c
lspscrip.o: lspscrip.c
