## Process this file with automake to produce Makefile.in
# ####################################################################
#
# ORIGINAL AUTHOR: Thomas Werne
# COMPANY: Jet Propulsion Laboratory
# VERSION:
#
# File Name:     Makefile.am
# Creation Date: 08 Oct 2012
#
# Copyright 2009-2012, by the California Institute of Technology.
# ALL RIGHTS RESERVED.  United States Government Sponsorship
# acknowledged. Any commercial use must be negotiated with the Office
# of Technology Transfer at the California Institute of Technology.
#
# This software may be subject to U.S. export control laws and
# regulations.  By accepting this document, the user agrees to comply
# with all U.S. export laws and regulations.  User has the
# responsibility to obtain export licenses, or other export authority
# as may be required before exporting such information to foreign
# countries or providing access to foreign persons.
# ####################################################################

########################################################################
# Setup
########################################################################
ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = @CFLAGS@ -D_NO_PROTO -Wno-deprecated -fPIC
AM_CXXFLAGS = @CXXFLAGS@ -D__SCAT_GNUCPP__ -DFUNCPROTO -D_REENTRANT -DXTFUNCPROTO -D__EXTENSIONS__ -DNOPM -Wno-deprecated

AM_CPPFLAGS = -I. -I@top_srcdir@/objs -I@top_srcdir@/eadata @CPPFLAGS@

AM_LDFLAGS =   \
    -lstdc++   \
    @LDFLAGS@

lib_LIBRARIES = \
    libEAData.a \
    libSim.a

LDADD = libSim.a libEAData.a

bin_PROGRAMS = \
    programs/baseband_errors                      \
    programs/BYUX_vs_time                         \
    programs/calpulse_to_ascii                    \
    programs/cds_tracking_check                   \
    programs/checkframe_extract                   \
    programs/checkframe_to_ascii                  \
    programs/check_inst_tracking                  \
    programs/dtc_format                           \
    programs/echo_deltaf                          \
    programs/echo_dtc                             \
    programs/echo_proc                            \
    programs/ephem_to_ascii                       \
    programs/eqx                                  \
    programs/expand_calpulse                      \
    programs/fake_s0_data                         \
    programs/fake_vap                             \
    programs/fit_dtc                              \
    programs/fix_ecmwf_speed                      \
    programs/gen_swath_e2b                        \
    programs/generate_dtc                         \
    programs/generate_rgc                         \
    programs/get_att                              \
    programs/get_DopRan_scan                      \
    programs/get_foff                             \
    programs/get_look_att                         \
    programs/get_X_scan                           \
    programs/gmf_plot                             \
    programs/gs_l1a_to_ascii                      \
    programs/kprtable_to_xmgr                     \
    programs/l00_to_l1a                           \
    programs/l1a_hdf_energy_map                   \
    programs/l1a_hdf_to_l1a                       \
    programs/l1a_to_ascii                         \
    programs/l1a_to_gs_l1a                        \
    programs/l1a_to_l1b                           \
    programs/l1b_ascii                            \
    programs/l1b_coverage                         \
    programs/l1bhdf_sigma0_histo                  \
    programs/l1b_hdf_to_l1b                       \
    programs/l1b_hdf_to_l1b_fast                  \
    programs/l1b_isro_to_l1b                      \
    programs/l1b_outlines                         \
    programs/l1b_s0                               \
    programs/l1b_to_ascii                         \
    programs/l1b_to_half_egg                      \
    programs/l1b_to_kprtable                      \
    programs/l1b_to_l2a                           \
    programs/l2ab_25to50                          \
    programs/l2a_centroids                        \
    programs/l2a_dissect                          \
    programs/l2a_filter                           \
    programs/l2a_fix_QS_composites                \
    programs/l2a_getcell                          \
    programs/l2ags_s0                             \
    programs/l2ags_s0l                            \
    programs/l2a_hdf_to_l2a                       \
    programs/l2a_maps                             \
    programs/l2a_s0                               \
    programs/l2a_s0l                              \
    programs/l2a_to_ascii                         \
    programs/l2a_to_l2b                           \
    programs/l2ax_hdf_to_l2a                      \
    programs/l2b_25km_splice                      \
    programs/l2b_diff                             \
    programs/l2b_diff_from_truth                  \
    programs/l2b_diff_from_truth_tmp              \
    programs/l2b_extract_worst                    \
    programs/l2b_get_diffs                        \
    programs/l2b_hdf_to_netcdf                    \
    programs/l2b_manyMet                          \
    programs/l2b_match_selected                   \
    programs/l2b_medianfilter                     \
    programs/l2b_metrics                          \
    programs/l2b_remove_extra                     \
    programs/l2b_spddif                           \
    programs/l2b_to_ascii                         \
    programs/l2b_to_flower                        \
    programs/l2b_to_netcdf                        \
    programs/l2b_to_retdat                        \
    programs/l2b_to_vctr                          \
    programs/l2_kp                                \
    programs/log_cfg                              \
    programs/make_kfactor                         \
    programs/make_kpmfield                        \
    programs/make_simple_land_map                 \
    programs/make_timetlm                         \
    programs/memfilter                            \
    programs/modu                                 \
    programs/mudh_class                           \
    programs/mudh_table                           \
    programs/os2_l2b_to_netcdf                    \
    programs/pl1a_to_ascii                        \
    programs/pl1a_to_l1b                          \
    programs/plot_obj_curves                      \
    programs/plot_solution_curves                 \
    programs/plot_wr_curves                       \
    programs/prob_3d                              \
    programs/psim                                 \
    programs/reformat_pattern                     \
    programs/rgc_delay_errors                     \
    programs/rgc_format                           \
    programs/s0_to_noise_value                    \
    programs/see_spot                             \
    programs/sim                                  \
    programs/sim_l1a_echo_proc                    \
    programs/sim_overlay                          \
    programs/smooth_kprtable                      \
    programs/SNRandKpc                            \
    programs/tc_info                              \
    programs/test_hdf_l2b                         \
    programs/test_pattern                         \
    programs/windfield_to_vctr                    \
    programs/xtable_to_xmgr                       \
    programs/RS_GSE_merge                         \
    programs/generate_dtc_from_ephem_quat_files   \
    programs/generate_xfactor_from_ephem_quat_files \
    programs/gen_os2_t2b12                        \
    programs/gen_os2_e2b12                        \
    programs/quat_to_gs_att                       \
    programs/ephem_to_gs                          \
    programs/ephem_quat_fill_gaps                 \
    programs/ovwm_sim                             \
    programs/hdf_l1a_writer                       \
    programs/gen_os2_l1b_fp_ecmwf                 \
    programs/gen_qs_l1b_fp_ecmwf

# other programs (requires external libraries)
# programs/generate_rgc_from_ephem_quat_files
# programs/RS_GSE_to_ephem_quat

########################################################################

libEAData_a_SOURCES =                \
    eadata/ArgDefs.h                 \
    eadata/Args.C                    \
    eadata/Args.h                    \
    eadata/ArgsPlus.C                \
    eadata/ArgsPlus.h                \
    eadata/Backup.C                  \
    eadata/Backup.h                  \
    eadata/Binning.C                 \
    eadata/Binning.h                 \
    eadata/CmdEffect.C               \
    eadata/CmdEffect.h               \
    eadata/CmdList.C                 \
    eadata/CmdList.h                 \
    eadata/Command_1.h               \
    eadata/Command.C                 \
    eadata/Command.h                 \
    eadata/CommonDefs.C              \
    eadata/CommonDefs.h              \
    eadata/Drn.C                     \
    eadata/Drn.h                     \
    eadata/DrvParamList.C            \
    eadata/DrvParamList.h            \
    eadata/EAConfigList.C            \
    eadata/EAConfigList.h            \
    eadata/EAList.C                  \
    eadata/EAList.h                  \
    eadata/EALog.C                   \
    eadata/EALog.h                   \
    eadata/EffDetector.C             \
    eadata/EffDetector.h             \
    eadata/Eqx.C                     \
    eadata/Eqx.h                     \
    eadata/ErrorChecker.C            \
    eadata/ErrorChecker.h            \
    eadata/Filter.C                  \
    eadata/FilterFunc.C              \
    eadata/FilterFunc.h              \
    eadata/Filter.h                  \
    eadata/FrameRead.C               \
    eadata/FrameRead.h               \
    eadata/HdfFile.C                 \
    eadata/HdfFile.h                 \
    eadata/Headers.C                 \
    eadata/Headers.h                 \
    eadata/Hk2EffDetector.C          \
    eadata/Hk2EffDetector.h          \
    eadata/Hk2ErrorChecker.C         \
    eadata/Hk2ErrorChecker.h         \
    eadata/Hk2Extract.C              \
    eadata/Hk2Extract.h              \
    eadata/Hk2File.C                 \
    eadata/Hk2File.h                 \
    eadata/Hk2FilTab.C               \
    eadata/Hk2HdfMap.C               \
    eadata/Hk2HdfMap.h               \
    eadata/Hk2ParTab.C               \
    eadata/HkdtExtract.h             \
    eadata/Itime.C                   \
    eadata/Itime.h                   \
    eadata/L1ADrvExtract.C           \
    eadata/L1ADrvExtract.h           \
    eadata/L1ADrvTab.C               \
    eadata/L1AEffDetector.C          \
    eadata/L1AEffDetector.h          \
    eadata/L1AErrorChecker.C         \
    eadata/L1AErrorChecker.h         \
    eadata/L1AExtract.C              \
    eadata/L1AExtract.h              \
    eadata/L1AFile.C                 \
    eadata/L1AFile.h                 \
    eadata/L1AFilTab.C               \
    eadata/L1AParTab.C               \
    eadata/L1BHdfFile.C              \
    eadata/L1BHdfFile.h              \
    eadata/L1BParTab.C               \
    eadata/L2AhrParTab.C             \
    eadata/L2AParTab.C               \
    eadata/L2AxParTab.C              \
    eadata/L2BParTab.C               \
    eadata/LeapSecTable.C            \
    eadata/LeapSecTable.h            \
    eadata/LimitChecker.C            \
    eadata/LimitChecker.h            \
    eadata/LimitList.C               \
    eadata/LimitList.h               \
    eadata/LimitState.C              \
    eadata/LimitState.h              \
    eadata/ListBase.C                \
    eadata/ListBase.h                \
    eadata/MacroCommand.h            \
    eadata/Mail.C                    \
    eadata/Mail.h                    \
    eadata/MemoryFromFile.C          \
    eadata/MemoryFromFile.h          \
    eadata/MemoryFromTlm.C           \
    eadata/MemoryFromTlm.h           \
    eadata/NoTimeTlmFile.C           \
    eadata/NoTimeTlmFile.h           \
    eadata/NSCAT.h                   \
    eadata/Parameter.C               \
    eadata/Parameter.h               \
    eadata/ParameterList.C           \
    eadata/ParameterList.h           \
    eadata/ParTab.C                  \
    eadata/ParTab.h                  \
    eadata/PolyErrNo.h               \
    eadata/Polynomial.C              \
    eadata/Polynomial.h              \
    eadata/PolyTable.C               \
    eadata/PolyTable.h               \
    eadata/Print.C                   \
    eadata/Print.h                   \
    eadata/Qpa.C                     \
    eadata/Qpa.h                     \
    eadata/Qpf.C                     \
    eadata/Qpf.h                     \
    eadata/Reqa.C                    \
    eadata/Reqa.h                    \
    eadata/ReqaqList.C               \
    eadata/ReqaqList.h               \
    eadata/Reqq.C                    \
    eadata/Reqq.h                    \
    eadata/Rtcf.C                    \
    eadata/Rtcf.h                    \
    eadata/SafeString.C              \
    eadata/SafeString.h              \
    eadata/State.C                   \
    eadata/State.h                   \
    eadata/TempInst.C                \
    eadata/TimeTlmFile.C             \
    eadata/TimeTlmFile.h             \
    eadata/TlmFileList.C             \
    eadata/TlmFileList.h             \
    eadata/TlmHdfFile.C              \
    eadata/TlmHdfFile.h              \
    eadata/ToDo.C                    \
    eadata/ToDo.h                    \
    eadata/Tpg.C                     \
    eadata/Tpg.h                     \
    eadata/UpldTbl_1.h               \
    eadata/UpldTbl.C                 \
    eadata/UpldTbl.h                 \
    eadata/UpldTblList.C             \
    eadata/UpldTblList.h


########################################################################

libSim_a_LIBADD = libEAData.a

libSim_a_SOURCES =                   \
    objs/AccurateGeom.C              \
    objs/AccurateGeom.h              \
    objs/AmbigTable.C                \
    objs/AmbigTable.h                \
    objs/AngleInterval.C             \
    objs/AngleInterval.h             \
    objs/Antenna.C                   \
    objs/Antenna.h                   \
    objs/Array.C                     \
    objs/Array.h                     \
    objs/AscatL1bReader.C            \
    objs/AscatL1bReader.h            \
    objs/AttenMap.C                  \
    objs/AttenMap.h                  \
    objs/Attitude.C                  \
    objs/Attitude.h                  \
    objs/BaseFile.C                  \
    objs/BaseFile.h                  \
    objs/Beam.C                      \
    objs/Beam.h                      \
    objs/BYUXTable.C                 \
    objs/BYUXTable.h                 \
    objs/CheckFrame.C                \
    objs/CheckFrame.h                \
    objs/CoastalMaps.C               \
    objs/CoastalMaps.h               \
    objs/ColorMap.C                  \
    objs/ColorMap.h                  \
    objs/ConfigList.C                \
    objs/ConfigList.h                \
    objs/ConfigSim.C                 \
    objs/ConfigSimDefs.h             \
    objs/ConfigSim.h                 \
    objs/Constants.h                 \
    objs/CoordinateSwitch.C          \
    objs/CoordinateSwitch.h          \
    objs/Distributions.C             \
    objs/Distributions.h             \
    objs/dot_prod.c                  \
    objs/doub_factorial_double.c     \
    objs/doub_factorial_int.c        \
    objs/EarthField.C                \
    objs/EarthField.h                \
    objs/EarthGeom.C                 \
    objs/EarthGeom.h                 \
    objs/EarthPosition.C             \
    objs/EarthPosition.h             \
    objs/ENOFTable.C                 \
    objs/ENOFTable.h                 \
    objs/Ephemeris.C                 \
    objs/Ephemeris.h                 \
    objs/erf_cheb.c                  \
    objs/erfcomp_cheb.c              \
    objs/ETime.C                     \
    objs/ETime.h                     \
    objs/factorial_double.c          \
    objs/factorial_int.c             \
    objs/FbbTable.C                  \
    objs/FbbTable.h                  \
    objs/Flower.C                    \
    objs/Flower.h                    \
    objs/function.h                  \
    objs/GenericGeom.C               \
    objs/GenericGeom.h               \
    objs/GeomNoiseFile.C             \
    objs/GeomNoiseFile.h             \
    objs/GMF.C                       \
    objs/GMF.h                       \
    objs/Grid.C                      \
    objs/Grid.h                      \
    objs/GSparameters.h              \
    objs/HdfHelp.C                   \
    objs/HdfHelp.h                   \
    objs/hdf_support.C               \
    objs/hdf_support.h               \
    objs/Image.C                     \
    objs/Image.h                     \
    objs/Index.C                     \
    objs/Index.h                     \
    objs/InstrumentGeom.C            \
    objs/InstrumentGeom.h            \
    objs/Interpolate.C               \
    objs/Interpolate.h               \
    objs/int_rn_exp.c                \
    objs/Jpeg.C                      \
    objs/Jpeg.h                      \
    objs/Kp.C                        \
    objs/Kp.h                        \
    objs/Kpm.C                       \
    objs/Kpm.h                       \
    objs/Kpr.C                       \
    objs/Kpr.h                       \
    objs/KpStatistics.C              \
    objs/KpStatistics.h              \
    objs/L00.C                       \
    objs/L00Frame.C                  \
    objs/L00Frame.h                  \
    objs/L00.h                       \
    objs/L00ToL1A.C                  \
    objs/L00ToL1A.h                  \
    objs/L1A.C                       \
    objs/L1AFrame.C                  \
    objs/L1AFrame.h                  \
    objs/L1AGSFrame.C                \
    objs/L1AGSFrame.h                \
    objs/L1A.h                       \
    objs/L1AH.C                      \
    objs/L1AHdf.C                    \
    objs/L1AHdf.h                    \
    objs/L1AH.h                      \
    objs/L1AToL1B.C                  \
    objs/L1AToL1B.h                  \
    objs/L1B.C                       \
    objs/L1B.h                       \
    objs/L1BHdf.C                    \
    objs/L1BHdf.h                    \
    objs/L1BToL2A.C                  \
    objs/L1BToL2A.h                  \
    objs/L2A.C                       \
    objs/L2A.h                       \
    objs/L2AH.C                      \
    objs/L2AHdf.C                    \
    objs/L2AHdf.h                    \
    objs/L2AH.h                      \
    objs/L2AToL2B.C                  \
    objs/L2AToL2B.h                  \
    objs/L2B.C                       \
    objs/L2B.h                       \
    objs/LandMap.C                   \
    objs/LandMap.h                   \
    objs/List.h                      \
    objs/LonLat.C                    \
    objs/LonLat.h                    \
    objs/LonLatWind.h                \
    objs/Malloc1.C                   \
    objs/Malloc1.h                   \
    objs/Mat.C                       \
    objs/Mat.h                       \
    objs/Matrix3.C                   \
    objs/Matrix3.h                   \
    objs/Matrix.C                    \
    objs/Matrix.h                    \
    objs/Meas.C                      \
    objs/Meas.h                      \
    objs/Metrics.C                   \
    objs/Metrics.h                   \
    objs/Misc.C                      \
    objs/Misc.h                      \
    objs/MiscTable.C                 \
    objs/MiscTable.h                 \
    objs/MLP.C                       \
    objs/MLPData.C                   \
    objs/MLPData.h                   \
    objs/MLP.h                       \
    objs/NetCDF_Attr.C               \
    objs/NetCDF_Attr.h               \
    objs/NetCDF.C                    \
    objs/NetCDF.h                    \
    objs/NetCDF_Var.C                \
    objs/NetCDF_Var.h                \
    objs/Options.C                   \
    objs/Options.h                   \
    objs/OS2XFix.C                   \
    objs/OS2XFix.h                   \
    objs/Ovwm.C                      \
    objs/OvwmConfig.C                \
    objs/OvwmConfigDefs.h            \
    objs/OvwmConfig.h                \
    objs/Ovwm.h                      \
    objs/OvwmL1A.C                   \
    objs/OvwmL1A.h                   \
    objs/OvwmL1AToL1B.C              \
    objs/OvwmL1AToL1B.h              \
    objs/OvwmSigma0.C                \
    objs/OvwmSigma0.h                \
    objs/OvwmSim.C                   \
    objs/OvwmSim.h                   \
    objs/PMeas.C                     \
    objs/PMeas.h                     \
    objs/PointList.C                 \
    objs/PointList.h                 \
    objs/PointTargetResponseTable.C  \
    objs/PointTargetResponseTable.h  \
    objs/Pscat.C                     \
    objs/PscatConfig.C               \
    objs/PscatConfigDefs.h           \
    objs/PscatConfig.h               \
    objs/Pscat.h                     \
    objs/PscatL1A.C                  \
    objs/PscatL1A.h                  \
    objs/PscatL1AToL1B.C             \
    objs/PscatL1AToL1B.h             \
    objs/PscatSim.C                  \
    objs/PscatSim.h                  \
    objs/Pulser.C                    \
    objs/Pulser.h                    \
    objs/Qscat.C                     \
    objs/QscatConfig.C               \
    objs/QscatConfigDefs.h           \
    objs/QscatConfig.h               \
    objs/Qscat.h                     \
    objs/QscatSim.C                  \
    objs/QscatSim.h                  \
    objs/qual_flag.c                 \
    objs/Quat.C                      \
    objs/Quat.h                      \
    objs/Rain.C                      \
    objs/RainDistribution.C          \
    objs/RainDistribution.h          \
    objs/Rain.h                      \
    objs/RainHeightTable.C           \
    objs/RainHeightTable.h           \
    objs/Scatterometer.C             \
    objs/Scatterometer.h             \
    objs/SchToXyz.C                  \
    objs/SchToXyz.h                  \
    objs/Sds.C                       \
    objs/Sds.h                       \
    objs/SeaPac.C                    \
    objs/SeaPac.h                    \
    objs/Sfile.C                     \
    objs/Sfile.h                     \
    objs/Sigma0.C                    \
    objs/Sigma0.h                    \
    objs/Sigma0Map.C                 \
    objs/Sigma0Map.h                 \
    objs/Spacecraft.C                \
    objs/Spacecraft.h                \
    objs/SpacecraftSim.C             \
    objs/SpacecraftSim.h             \
    objs/sphere_normal_prob.c        \
    objs/stirling4.c                 \
    objs/SwapEndian.c                \
    objs/SwapEndian.h                \
    objs/Topo.C                      \
    objs/Topo.h                      \
    objs/Tracking.C                  \
    objs/Tracking.h                  \
    objs/Transform.C                 \
    objs/Transform.h                 \
    objs/Utils.C                     \
    objs/Utils.h                     \
    objs/Vect.C                      \
    objs/Vect.h                      \
    objs/Wind.C                      \
    objs/WindGrad.C                  \
    objs/WindGrad.h                  \
    objs/Wind.h                      \
    objs/WindSwath.C                 \
    objs/WindSwath.h                 \
    objs/WindVector.C                \
    objs/WindVector.h                \
    objs/XTable.C                    \
    objs/XTable.h                    \
    objs/YahyaAntenna.h

########################################################################
#ParameterList.o
#	$(CC) -c $(CDEBUGFLAGS) -D__SCAT_GNUCPP__ -DFUNCPROTO -D$(DEBUG) \
#	-D_REENTRANT -DXTFUNCPROTO -D__EXTENSIONS__ \
#	$(INCFLAGS) -DNOPM -fPIC ParameterList.C
