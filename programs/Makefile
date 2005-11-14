#======================================================================
# @(#) $Id$
# Makefile for RAS programs
#======================================================================

SHELL = /bin/csh
INCDIR = ../../include
INCFLAGS = -I$(INCDIR) -I$(SPICEDIR) -I.
DEBUGFLAGS = -ggdb2  

SPICELIB = $(SPICEDIR)/../../lib/cspice.a
LIBS = ../../lib/libsupport.a $(SPICELIB)
LDLIBS = $(LIBS) -lm
INSTALLDIR = ../../bin

CPPFLAGS = $(SPECIAL_USER_FLAGS) $(DEBUGFLAGS) $(INCFLAGS) -Wall
FFLAGS = -ffixed-line-length-none
SYSLIBS = -lm
CCC = g++
CC = gcc
FF = g77

# core of uplink processing
MAIN_UPLINK = \
  dlap_generate \
  ckernel_write \
  config_merge \
  wtk2seqsasf \
  main_rmss\
  set_latlon_target\
  select_prf_csf 

# LIVE UPDATE software
LIVE_UPDATE = \
  compare_spk_files \
  live_ivp_update 

# uplink testing software
UPLINK_TEST = \
  kernel_display \
  ieb_param_display \
  performance_cal \
  performance_display \
  check_5dB_contiguity \
  amb_show \
  compare_iebs \
  doppler_profile \
  compare_ckernels \
  ieb_param_generate

# Command Design tools
DESIGN = \
  amb_param_design \
  amb_geom_cal \
  amb_param_scan \
  minimize_gap \
  geom_cal_amb \
  param_scan_script \
  geom_cal_script \
  prf_hop \
  ras_noise_cal \
  distant_scat\
 show_alt_track

# Main downlink software
MAIN_DOWNLINK = \
  l0_l1b\
  preproc \
  sar_proc \
  rad_proc \
  scatt_proc\
  alt_proc\
  esb_cal \
  generate_radio_map

# Downlink support software
DOWNLINK_SUPPORT = \
  generate_geometry_file \
  get_gaincutoff_coeffs \
  rad_cal \
  mw_cal \
  show_dop_centroid\
  display_ypaz_fit\
  track_int_doppler\
  reconstruct_ckernel \
  ras_sim \
  clip_header \
  lbdr_sim \
  cut_binary_record \
  sar_index_to_dopran \
  find_pixel \
  getint_from_bidr \
  getfloat_from_bidr \
  realtocomplex \
  rowcol_from_backplane \
  fix_incomplete_lbdr \
  mean_std_from_bidr\
  compare_lbdrs\
  compute_noise \
  compressed_scatt_proc\
  show_histogram

# Downlink testing software
DOWNLINK_TEST = \
  burst_data_extract \
  point_target_sim \
  monitor \
  state_vector_extract \
  compare_BODP_to_SIS \
  echo_sim \
  bidr_slide_show \
  compare_l1b_echoes \
  read_lbdr \
  read_sbdr\
  compute_spcal_debug \
  compute_sys_temp \
  collect_rad_data\
  generate_map\
  show_spectrum \
  tdc_proc \
  generate_map_file \
  performance_evaluation 

# Radar analysis Software
RAS = \
  baqview \
  parview \
  rngview \
  ras_check \
  lbdr_check \
  ras_echo_show \
  l0_echo_plot \
  lbdr_echo_show \
  calview 
 
  
# BEAM PATTERN GENERATION SOFTWARE
BEAMPAT = \
  convert_beam_file \
  beam_test \
  create_sunscan_file \
  beam_to_beam_offset \
  get_beam_profiles 

# Miscellaneous
MISC = \
  self_tests \
  getClosestApproach \
  doppler_plot \
  sim_ephem_error \
  plot_point_stats \
  check_ckernel \
  get_altitude \
  target_rotation \
  debug_spice \
  contiguity \
  generate_sim_range_doppler_file \
  generate_range_doppler_file \
  generate_ypaz_ck 

  

ALL = \
         $(MAIN_UPLINK) $(LIVE_UPDATE) $(UPLINK_TEST) \
         $(DESIGN) $(MAIN_DOWNLINK) $(DOWNLINK_SUPPORT) $(DOWNLINK_TEST) \
         $(RAS) $(BEAMPAT) $(MISC)

.PRECIOUS: %.cpp
.PRECIOUS: %.F
.SUFFIXES: .cpp
.SUFFIXES: .F

# Implicit rules

.cpp.o:
	$(CC) $(CPPFLAGS) $<

.F.o:
	$(FF) $(FFLAGS) $<

# High level make targets

default: getopt_special.h $(ALL)
up: getopt_special.h $(MAIN_UPLINK) $(LIVE_UPDATE) $(UPLINK_TEST)
up_main: getopt_special.h $(MAIN_UPLINK)
up_live: getopt_special.h $(LIVE_UPDATE)
up_test: getopt_special.h $(UPLINK_TEST)
down: getopt_special.h $(MAIN_DOWNLINK) $(DOWNLINK_SUPPORT) $(DOWNLINK_TEST)
ras: getopt_special.h $(RAS)
other: getopt_special.h $(DESIGN) $(BEAMPAT) $(MISC)
bad:  getopt_special.h $(BAD)

# Explicit rules

%.cpp: $(RAS_CENTRAL_TREE)/src/programs/RCS/%.cpp,v 
	co $(RAS_CENTRAL_TREE)/src/programs/RCS/$@,v

%.F: $(RAS_CENTRAL_TREE)/src/programs/RCS/%.F,v 
	co $(RAS_CENTRAL_TREE)/src/programs/RCS/$@,v

%.h: $(RAS_CENTRAL_TREE)/src/programs/RCS/%.h,v
	co $(RAS_CENTRAL_TREE)/src/programs/RCS/$@,v

%: %.cpp $(LIBS) 
	$(CCC) $(CPPFLAGS) -o $@ $@.cpp $(LDLIBS)

%: %.F  
	$(FF) $(FFLAGS) -o $@ $@.F 

#	$(FF) -force_flat_namespace -o $@ $@.o $(LDLIBS)

install: 
	mv -f $(ALL) $(INSTALLDIR)

clean:
	- /bin/rm -f $(ALL)
	- /bin/rm -f core



