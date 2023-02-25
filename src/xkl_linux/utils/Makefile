# $Log: Makefile,v $
# Revision 1.6  2001-07-18 16:18:37+00  tiede
# ms2kl, kl2ms now compiled PROGS; added sgisd2kl to SCRIPTS
#
# Revision 1.5  2000/04/20 05:32:38  krishna
# Changed install to copy executables instead of moving them.
#
# Revision 1.4  1998/07/22 14:31:04  krishna
# Added ms2kl and kl2ms.
#

#######################################################################
#
# Makefile for utils.  No need fo user to edit in here.
#
#######################################################################

include ../Makefile.vars

PROGS = concat kl2raw klinfo klplay klrecord raw2kl klgain ms2kl kl2ms
SCRIPTS = kl2sd sd2kl sgikl2sd sgisd2kl mac2kl kl2mac
MATLAB_SCRIPTS = mat2kl.m kl2mat.m

INCLUDES = -I../common

OBJS = raw2kl.o kl2raw.o klinfo.o klplay.o klrecord.o concat.o klgain.o ms2kl.o kl2ms.o

COMMON_OBJS = ../common/wavio.o ../common/getargs.o

all: $(PROGS) $(SCRIPTS)

install:
	chmod go+rx $(PROGS) $(SCRIPTS)
	cp -f $(PROGS) $(SCRIPTS) $(BINDIR)
	cp -f $(MATLAB_SCRIPTS) $(MATLAB_DIR)

clean:	
	rm -f *.o core *~ $(PROGS) $(OBJS)

concat: concat.o $(COMMON_OBJS)
	${CC} ${CFLAGS} -o $@ concat.o $(COMMON_OBJS) -lm

raw2kl: raw2kl.o $(COMMON_OBJS)
	${CC} ${CFLAGS} -o $@ raw2kl.o $(COMMON_OBJS) -lm

kl2raw: kl2raw.o $(COMMON_OBJS)
	${CC} ${CFLAGS} -o $@ kl2raw.o $(COMMON_OBJS) -lm

ms2kl: ms2kl.o $(COMMON_OBJS)
	${CC} ${CFLAGS} -o $@ ms2kl.o $(COMMON_OBJS) -lm

kl2ms: kl2ms.o $(COMMON_OBJS)
	${CC} ${CFLAGS} -o $@ kl2ms.o $(COMMON_OBJS) -lm

klinfo: klinfo.o $(COMMON_OBJS) ../common/wavdata.o
	${CC} ${CFLAGS} -o $@ klinfo.o $(COMMON_OBJS) \
		../common/wavdata.o -lm

klgain: klgain.o $(COMMON_OBJS) ../common/wavdata.o
	${CC} ${CFLAGS} -o $@ klgain.o \
		$(COMMON_OBJS) ../common/wavdata.o -lm

klplay: klplay.o $(COMMON_OBJS) ../common/wavaudio.o
	${CC} ${CFLAGS} -o $@ klplay.o $(COMMON_OBJS) \
		../common/wavaudio.o -lm

klrecord: klrecord.o $(COMMON_OBJS) ../common/wavaudio.o
	${CC} ${CFLAGS} -o $@ klrecord.o $(COMMON_OBJS) \
		 ../common/wavaudio.o  -lm
