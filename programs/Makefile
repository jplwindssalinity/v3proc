#======================================================================
# @(#) $Id$
# Makefile for all SIM programs
#======================================================================

SHELL =			/bin/csh
INCDIR =		../../include
HDFINCDIR =		/usr/local/hdf/include

EALIBS =		../../lib/libEAData.a /usr/local/lib/libtiff.a
HDFLIBS =		-L/usr/local/hdf/lib -lmfhdf -ldf -lz -lnsl
LIBS =			../../lib/libSim.a
LDLIBS =		$(LIBS) $(EALIBS) $(HDFLIBS) -lm -lstdc++ -lsocket
PURE =

CCC =			$(PURE) gcc
DEBUGFLAGS =	-O2
CPPFLAGS =		$(DEBUGFLAGS) -Wall -I$(INCDIR) -I$(HDFINCDIR) -DNOPM

INSTALLDIR =	../../bin

PROGRAMS = \
	sim \
	l1a_to_l1b \
	l1b_to_l2a \
	l2a_to_l2b \
	l2b_metrics \
	l2b_super_metrics \
	l1b_coverage \
	l2a_hdf_to_l2a \
	l2ax_hdf_to_l2a \
	l2b_medianfilter \
	l2a_filter \
	l1b_ascii \
	l2a_s0 \
	l2b_to_vctr \
	sim_overlay \
	modu \
	echo_dtc \
	fit_dtc \
	get_att \
	get_look_att \
	echo_proc \
	psim \
	pl1a_to_l1b \
	pl1a_to_ascii \
	l1a_hdf_to_l1a \
	plot_wr_curves \
	new_files \
	done_files \
	l2ab_25to50 \
	l2b_diff \
	l2b_diff_from_truth \
	test_hdf_l2b \
	l2b_manyMet \
	l2b_match_selected \
	see_spot \
	eqx \
	generate_rgc \
	generate_dtc \
	rgc_delay_errors \
	rgc_format \
	dtc_format \
    tc_info \
	cds_tracking_check \
	memfilter \
	fake_vap \
	l1b_hdf_to_l1b \
    l1bhdf_sigma0_histo \
	l1b_outlines \
	l1b_to_kprtable \
	make_kfactor \
	make_kpmfield \
	windfield_to_vctr \
	l2a_centroids \
	l2a_maps \
	l2_kp \
	l2a_s0l \
	l2ags_s0l \
	smooth_kprtable \
	xtable_to_xmgr \
	l1b_s0 \
	kprtable_to_xmgr \
	s0_to_noise_value \
	log_cfg \
	l2ags_s0 \
	fix_ecmwf_speed \
	gs_l1a_to_ascii \
	l2b_to_flower \
	l2a_dissect \
	l2a_getcell \
	l2b_spddif \
	l2b_get_diffs \
	prob_3d \
	SNRandKpc \
	plot_obj_curves \
	l1b_to_half_egg \
	l1a_to_gs_l1a\
	l1a_to_ascii\
	l1b_to_ascii \
	checkframe_to_ascii \
    checkframe_extract \
	l2a_to_ascii \
	l2b_to_ascii \
	reformat_pattern \
	test_pattern \
	get_DopRan_scan \
	get_X_scan \
    expand_calpulse \
    ephem_to_ascii \
	fake_s0_data \
	gmf_plot \
	BYUX_vs_time \
    calpulse_to_ascii \
    make_timetlm \
	l2b_extract_worst \
	get_foff \
	make_simple_land_map
#	l00_to_l1a
#   check_l1b 
#   nof_and_qual
#	gen_foff
#	sim_accurate
#	generate_kfactor
#	hdf_l1a_echo_proc
#	fix_knowledge
#	sim_l1a_echo_proc
#	echo_split
#	baseband_errors

SVT_PROGRAMS = \
    l1a_hdf_energy_map

STILES_PROGRAMS = \
	l2b_25km_splice \
	l1b_to_slicebal \
	l2b_remove_extra

HUDD_PROGRAMS = \
	sim \
	l00_to_l1a \
	l1a_to_l1b \
	l1b_to_l2a \
	l2a_to_l2b \
	l2b_to_vctr \
	l2b_super_metrics \
	l1b_ascii \
	see_spot \
	eqx \
	generate_rgc \
	generate_dtc \
	rgc_delay_errors \
	baseband_errors \
	rgc_format \
	dtc_format \
	fake_vap \
	plot_solution_curves \
	windfield_to_vctr \
	log_cfg \
	fix_ecmwf_speed \
	l2b_to_flower

DIAGNOSTIC_PROGRAMS = \
	l2ab_obj_comp \
    check_l1b \
	l2b_to_vctr \
	l2b_metrics \
	see_spot \
	test_pattern \
	l1b_ascii \
	l1b_outlines \
	plot_solution_curves \
	windfield_to_vctr \
	checkframe_to_ascii \
	l2a_centroids \
	l2a_maps \
	l2_kp \
	l2a_s0 \
	l2a_s0l \
	l2ags_s0l \
	xtable_to_xmgr \
	l1b_s0 \
	kprtable_to_xmgr \
	s0_to_noise_value \
	log_cfg \
	l2ags_s0 \
	l2b_to_flower \
	l2a_dissect \
	l1b_to_ascii \
    ephem_to_ascii \
	l2a_to_ascii

GS_CHECK_PROGRAMS = \
	sim \
    l1a_to_l1b \
    l1a_to_gs_l1a \
    l1b_hdf_to_l1b \
    checkframe_extract \
    checkframe_to_ascii \
    gs_l1a_to_ascii \
    l1a_to_ascii \
    calpulse_to_ascii \
    l1b_to_ascii
#   check_l1b 

#----------------------------------------------------------------------
# default: make all executable programs
#----------------------------------------------------------------------

default: $(PROGRAMS) $(GS_CHECK_PROGRAMS)
svt: $(SVT_PROGRAMS)
hudd: $(HUDD_PROGRAMS)
stiles: $(STILES_PROGRAMS)
diag: $(DIAGNOSTIC_PROGRAMS)
gschk: $(GS_CHECK_PROGRAMS)

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

%.C: $(SIM_CENTRAL_TREE)/src/programs/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/programs/RCS/$@,v

%.h: $(SIM_CENTRAL_TREE)/src/programs/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/programs/RCS/$@,v

%: %.C $(LIBS) $(EALIBS)
	$(CCC) $(CPPFLAGS) -o $@ $@.C $(LDLIBS)

fix_knowledge: fix_knowledge.C echo_funcs.h
echo_dtc: echo_dtc.C echo_funcs.h
sim_l1a_echo_proc: sim_l1a_echo_proc.C echo_funcs.h
new_files: new_files.C new_files.h
modu: modu.C modu.h
mudh_table: mudh_table.C mudh.h
mudh_class: mudh_class.C mudh.h
