#======================================================================
# @(#) $Id$
# Makefile for all SIM programs
#======================================================================

SHELL =			/bin/csh
INCDIR =		../../include
LIBS =			../../lib/libSim.a
LDLIBS =		$(LIBS) -lm
CCC =			gcc

CPPFLAGS =		-g -Wall -I$(INCDIR)

INSTALLDIR =	../../bin

PROGRAMS = \
	sim \
	l00_to_l10 \
	l10_to_l15 \
	l15_to_l17 \
	l17_to_l20 \
	l20_to_vctr \
	l20_metrics \
	see_spot \
	generate_rgc \
	generate_dtc \
	rgc_delay_errors \
	baseband_errors \
	reformat_pattern \
	memfilter \
	make_pattern \
	test_pattern \
	fake_vap \
	l15_outlines \
	plot_solution_curves \
	sim_accurate \
	make_kfactor \
	log_cfg \
	windfield_to_vctr \
	l17_centroids

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

%.C: $(SIM_CENTRAL_TREE)/src/programs/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/programs/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/programs/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/programs/RCS/$@,v

%: %.C $(LIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)
