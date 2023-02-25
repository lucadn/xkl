# $Log: Makefile,v $
# Revision 1.8  2002-06-13 11:07:38-04  tiede
# added xkl_dlog, xkl_label
#
# Revision 1.7  2000/09/15 20:24:22  tiede
# spectrogram now displayed correctly regardless of screen depth
# mods to support LEX *.label files
#
# Revision 1.6  2000/08/24 19:39:54  tiede
# platform specific data now loaded from Makefile.${PLATFORM}
#
# Revision 1.5  2000/06/08 03:10:58  tiede
# put under RCS
#
# Revision 1.4  2000/04/20 05:23:43  krishna
# Changed install to copy executables instead of moving them.
#
# Revision 1.3  1999/06/19 04:32:30  krishna
# Incremented Version.
#
# Revision 1.2  1998/07/17 08:02:30  krishna
# Added RCS header, and modified syn.o so it compiles using XKLDIR.
#

############################################################################
#
# Makefile for xkl
#
############################################################################

include ../Makefile.vars

XKL_VERSION = 3.2
XKL = xkl-$(XKL_VERSION)

PROGS = $(XKL)

LIB_PROGS = default60.con

# The objects

XKL_SRCS =     xkl.c xspec_util.c xinfo.c xklspec.c xkl_dlog.c xkl_label.c
XKL_OBJS =     xkl.o xspec_util.o xinfo.o xklspec.o xkl_dlog.o xkl_label.o
SYN_OBJS =     ../syn/syn.o ../syn/synparwav.o ../syn/syngrid.o \
		../syn/synscrip.o
COMMON_OBJS =  ../common/wavio.o ../common/spectrum.o \
		../common/wavaudio.o
OBJS = $(XKL_OBJS) $(SYN_OBJS) $(COMMON_OBJS)

# Process the includes

XINCS_DIR = $(XINCS_ARCH) -I../common
INCLUDES = -I/usr/include $(XINCS_DIR) $(XTRA_INCS_DIR) $(XTRA_INCS)

# Process the libraries

XLIBS_DIR = $(XTRA_LIBS_DIR)
#LIBS = $(XLIBS_DIR) -lXmu -lXm -lXt -lX11 -lm  $(XLIBS_ARCH) $(XTRA_LIBS)
LIBS = $(XLIBS_DIR) $(XLIBS_ARCH) $(XTRA_LIBS)

####################################################################

all: $(PROGS)

$(XKL): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

install:
	chmod go+rx $(PROGS)
	cp -f $(PROGS) $(SCRIPTS) $(BINDIR)
	ln -s $(BINDIR)/$(XKL) $(BINDIR)/xkl
	cp -f $(LIB_PROGS) $(LIBDIR)

clean:
	rm -f $(OBJS)

realclean:
	rm -f $(OBJS) $(PROGS)

xspec_util.o:  xspec_util.h xklspec.h xinfo.h version.h
xklspec.o:     xspec_util.h xklspec.h xinfo.h
xkl.o:         xspec_util.h xklspec.h xinfo.h xklsyn.h resources.h
xkl_dlog.o:    xspec_util.h
xkl_label.o:   xspec_util.h
../syn/syn.o: ../syn/syn.c ../common/wavio.h
	$(CC) -DXKLDIR=\"${LIBDIR}\" ${INCLUDES} -c -o $@ ../syn/syn.c

