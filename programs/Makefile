#======================================================================
# @(#) $Id$
# Makefile for all SIM programs
#======================================================================

SHELL =			/bin/csh
INCDIR =		../../include
LIBS =			../../lib/libSim.a
LDLIBS =		$(LIBS) -lm
CCC =			gcc
DEBUGFLAGS=              -O3
CPPFLAGS =		$(DEBUGFLAGS) -Wall -I$(INCDIR)

INSTALLDIR =	../../bin

PROGRAMS = \
	sim \
	l00_to_l1a \
	l1a_to_l1b \
	l1b_to_l2a \
	l2a_to_l2b \
	l2b_to_vctr \
	l2b_metrics \
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
	l1b_outlines \
    l1b_to_kprtable \
	plot_solution_curves \
	sim_accurate \
	make_kfactor \
	generate_kfactor \
	make_kpmfield \
	log_cfg \
	windfield_to_vctr \
	l2a_centroids \
	l2a_diagnostics \
	l2_kp \
	l2a_s0 \
	l1b_s0 \
        kprtable_to_xmgr \
        l2ags_s0


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






