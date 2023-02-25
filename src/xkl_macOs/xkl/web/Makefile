# $Log: Makefile.sav,v $
# Revision 1.2  1998/07/17 08:02:30  krishna
# Added RCS header, and modified syn.o so it compiles using XKLDIR.
#

############################################################################
#
# Makefile for xkl.
#
############################################################################

include ../../Makefile.vars

XKL_VERSION = 2.5
XKL = xkl-web

PROGS = $(XKL)

LIB_PROGS = default60.con

# The objects

XKL_SRCS =     web.c xspec_util.c 
XKL_OBJS =     web.o xspec_util.o 
SYN_OBJS =     ../../syn/syn.o ../../syn/synparwav.o ../../syn/syngrid.o \
		../../syn/synscrip.o
COMMON_OBJS =  ../../common/wavio.o ../../common/spectrum.o \
		../../common/wavaudio.o
OBJS = $(XKL_OBJS) $(SYN_OBJS) $(COMMON_OBJS)

# Process the includes

XINCS_DIR = -I/usr/X11/include -I/usr/include/X11 $(XINCS_ARCH) -I../common
INCLUDES = -I/usr/include $(XINCS_DIR) $(XTRA_XINCS_DIR) $(XTRA_XINCS)

# Process the libraries

XLIBS_DIR = -L/usr/X11/lib -L/usr/lib/X11 $(XTRA_LIBS_DIR)
LIBS = $(XLIBS_DIR) -lXmu -lXm -lXt -lX11 -lm  $(XLIBS_ARCH) $(XTRA_LIBS)

####################################################################

all: $(PROGS)

$(XKL): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

install:
	chmod go+rx $(PROGS)
	mv -i $(PROGS) $(SCRIPTS) $(BINDIR)
	ln -s $(BINDIR)/$(XKL) $(BINDIR)/xkl
	cp $(LIB_PROGS) $(LIBDIR)

clean:
	rm -f $(OBJS) $(PROGS)

web.o:         xspec_util.h web.h
xspec_util.o:  xspec_util.h xklspec.h xinfo.h
../../syn/syn.o: ../../syn/syn.c ../../common/wavio.h
	$(CC) -DXKLDIR=\"${LIBDIR}\" ${INCLUDES} -c -o $@ ../../syn/syn.c







