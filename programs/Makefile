#======================================================================
# @(#) $Id$
# Makefile for all SIM programs
#======================================================================

SHELL =			/bin/csh
INCDIR =		../../include
LIBS =			../../lib/libSim.a
LDLIBS =		$(LIBS) -lm

CPPFLAGS =		-g -ptr. -ptr../objs +w -I$(INCDIR)

INSTALLDIR =	../../bin

PROGRAMS = \
	l0_to_l1 \
	l1_to_l15 \
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

%.C: $(SIM_CENTRAL_TREE)/src/programs/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/programs/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/programs/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/programs/RCS/$@,v

%: %.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

sim: sim.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

l0_to_l1: l0_to_l1.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

l1_to_l15: l1_to_l15.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)
