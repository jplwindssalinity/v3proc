#======================================================================
# @(#) $Id$
# Makefile for EA programs
#======================================================================

SHELL =			/bin/csh
INCDIR =		../../include
HDFINCDIR =		$(INCDIR)/hdf

EALIBS =		../../lib/libEAData.a
HDFLIBS =		-L../../lib -lmfhdf -ldf -lz -lnsl
LIBS =			../../lib/libSim.a
LDLIBS =		$(LIBS) $(EALIBS) $(HDFLIBS) -lm -lstdc++
PURE =

CCC =			$(PURE) gcc
DEBUGFLAGS =	-O3
CPPFLAGS =		$(DEBUGFLAGS) -Wall -I$(INCDIR) -I$(HDFINCDIR) -DNOPM

INSTALLDIR =	../../bin

PROGRAMS = \
	qs_ea_extract \
	qs_ea_list_parameters

#----------------------------------------------------------------------
# default: make all executable programs
#----------------------------------------------------------------------

default: $(PROGRAMS)

#----------------------------------------------------------------------
# clean: remove all executable programs
#----------------------------------------------------------------------

clean:
	- /bin/rm -f $(PROGRAMS)

#----------------------------------------------------------------------
# install: make all object and executable programs and install
#----------------------------------------------------------------------

install: $(PROGRAMS)
	cp $(PROGRAMS) $(INSTALLDIR)

#----------------------------------------------------------------------
# rules
#----------------------------------------------------------------------

.PRECIOUS: %.C

%.C: $(SIM_CENTRAL_TREE)/src/eaprograms/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/eaprograms/RCS/$@,v

%.h: $(SIM_CENTRAL_TREE)/src/eaprograms/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/eaprograms/RCS/$@,v

%: %.C $(LIBS) $(EALIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

qs_ea_extract: extract.C Application.h
	$(CCC) $(CPPFLAGS) -o $@ extract.C $(LDLIBS)

qs_ea_list_parameters: list_parameters.C
	$(CCC) $(CPPFLAGS) -o $@ list_parameters.C $(LDLIBS)
