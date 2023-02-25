# Linux-specific XKL variables

include Makefile.vars


#----------------------------------------------------------------------
# Linux:
#
XINCS_ARCH =
XLIBS_ARCH = -lm -lXmu -lXm -lXpm -lXext -lXt -lX11 -lSM -lICE

#----------------------------------------------------------------------
# Static linked xkl (for Linux):
#
# XINCS_ARCH =
# XLIBS_ARCH = -lXmu -lXm -lm -lXpm -lXp -lXext -lXt -lX11 -lSM -lICE 


DIRS = common syn lspecto utils xkl

all::
	for dir in ${DIRS}; do \
	echo "Building in $$dir..."; \
	cd $$dir; ${MAKE} all; cd ..; \
	done

install::
	for dir in ${DIRS}; do \
	echo "Installing from $$dir..."; \
	cd $$dir; ${MAKE} install; cd ..; \
	done

clean::
	@for dir in ${DIRS}; do \
	echo "Cleaning $$dir..."; \
	cd $$dir; ${MAKE} clean; cd ..; \
	done
