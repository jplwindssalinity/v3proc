#======================================================================
# @(#) $Id$
# Makefile for all SIM programs
#======================================================================

SHELL =			/bin/csh
INCDIR =		../../include
LIBS =			../../lib/libSim.a
LDLIBS =		$(LIBS) -lm
CCC =			gcc

CPPFLAGS =		-g -pg -Wall -I$(INCDIR)

INSTALLDIR =	../../bin

PROGRAMS = \
	sim \
	l00_to_l10 \
	l10_to_l15 \
	l15_to_l17 \
	l17_to_l20

#----------------------------------------------------------------------
# default: make all executable programs
#----------------------------------------------------------------------

default: $(PROGRAMS)

#----------------------------------------------------------------------
# clean: remove all executable programs
#----------------------------------------------------------------------

clean:
	- rm -f $(PROGRAMS)

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

l00_to_l10: l00_to_l10.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

l10_to_l15: l10_to_l15.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

l15_to_l17: l15_to_l17.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

l17_to_l20: l17_to_l20.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)
