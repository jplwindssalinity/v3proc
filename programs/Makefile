#======================================================================
# @(#) $Id$
# Makefile for all SIM programs
#======================================================================

SHELL =			/bin/csh
INCDIR =		../../include
SIMLIB =		../../lib/libSim.a
SYSLIBS = 		-lm

CPPFLAGS =		-g -ptr. -ptr../objs +w -I$(INCDIR) $(SIMLIB) $(SYSLIBS)

.SUFFIXES = .C

INSTALLDIR =	../../bin

PROGRAMS = \
	sim

#----------------------------------------------------------------------
# default: make all executable programs
#----------------------------------------------------------------------

default: $(PROGRAMS)

#----------------------------------------------------------------------
# clean: remove all executable programs
#----------------------------------------------------------------------

clean:
	- rm -f $(PROGRAMS)
	- rm -rf Templates.DB

#----------------------------------------------------------------------
# install: make all object and executable programs and install
#----------------------------------------------------------------------

install: $(PROGRAMS)
	cp $(PROGRAMS) $(INSTALLDIR)

#----------------------------------------------------------------------
# rules
#----------------------------------------------------------------------
