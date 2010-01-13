#======================================================================
# @(#) $Id$
# Makefile for all SIM object code
#======================================================================

SHELL =			/bin/csh

INCDIR =		../../include
TARGET_LIB =	../../lib/libSim.a
HDFINCDIR =     /sw/include
MALLOC_DIR =    /usr/include/malloc
DEBUGFLAGS =    -ggdb2 --save-temps 
#DEBUGFLAGS =    -O2
INCFLAGS =      -I$(INCDIR) -I$(HDFINCDIR) -I$(MALLOC_DIR) -I.

# CPPFLAGS =		$(DEBUGFLAGS) $(INCFLAGS) -fno-implicit-templates -Wall -Wshadow -Woverloaded-virtual
CPPFLAGS =		$(DEBUGFLAGS) $(INCFLAGS) -fno-implicit-templates -Wall
SYSLIBS =		-lm
CCC =			gcc
CC  =			gcc

CPINCFILES =    echo *.h
C_OBJS = \
       dot_prod.o \
       erfcomp_cheb.o \
       qual_flag.o \
       doub_factorial_double.o  \
       factorial_double.o \
       sphere_normal_prob.o \
       doub_factorial_int.o \
       factorial_int.o    \
       stirling4.o \
       erf_cheb.o  \
       int_rn_exp.o

OBJS = \
	List.o \
	BufferedList.o \
	AccurateGeom.o \
	AmbigTable.o \
	Antenna.o \
	Array.o \
	AngleInterval.o \
	Attitude.o \
	BaseFile.o \
	Beam.o \
	BYUXTable.o \
	CheckFrame.o \
	CoastalMaps.o \
	ColorMap.o \
	ConfigList.o \
	ConfigSim.o \
	CoordinateSwitch.o \
	Distributions.o \
	EarthField.o \
	EarthGeom.o \
	EarthPosition.o \
	Ephemeris.o \
	ETime.o \
	FbbTable.o \
	Flower.o \
	GMF.o \
	GenericGeom.o \
	GeomNoiseFile.o \
	Grid.o \
	HdfHelp.o \
	hdf_support.o \
	Image.o \
	Index.o \
	InstrumentGeom.o \
	Interpolate.o \
	Jpeg.o \
	Kp.o \
	KpStatistics.o \
	Kpm.o \
	Kpr.o \
	LandMap.o \
	L00.o \
	L00Frame.o \
	L00ToL1A.o \
	L1A.o \
	L1AH.o \
	L1AFrame.o \
	L1AGSFrame.o \
	L1AHdf.o \
	L1AToL1B.o \
	L1B.o \
	L1BHdf.o \
	L1BToL2A.o \
	L2A.o \
	L2AH.o \
	L2AHdf.o \
	L2AToL2B.o \
	L2B.o \
	LonLat.o \
	Malloc1.o \
	Mat.o \
	Matrix.o \
	Matrix3.o \
	Meas.o \
	Metrics.o \
	Misc.o \
	MiscTable.o \
	MLP.o \
	MLPData.o\
	PMeas.o \
	PointList.o \
	PointTargetResponseTable.o \
	Pscat.o \
	PscatConfig.o \
	PscatL1A.o \
	PscatL1AToL1B.o \
	PscatSim.o \
	Pulser.o \
	Qscat.o \
	QscatConfig.o \
	QscatSim.o \
	Ovwm.o \
	OvwmL1A.o \
	OvwmL1AToL1B.o \
	OvwmConfig.o \
	OvwmSim.o \
	OvwmSigma0.o \
	Quat.o \
        Rain.o \
	RainDistribution.o \
	RainHeightTable.o \
	Scatterometer.o \
	SchToXyz.o \
	Sds.o \
	Sigma0.o \
	Sigma0Map.o \
	Spacecraft.o \
	SpacecraftSim.o \
	Sfile.o \
	Topo.o \
	Tracking.o \
	Transform.o \
	Vect.o \
	Wind.o \
	WindSwath.o \
	WindVector.o \
	WindGrad.o \
	XTable.o \
	SeaPac.o \
	ENOFTable.o \
	Options.o \
	Utils.o \
	AscatL1bReader.o

INCS = \
	*.h \
	List.C \
	BufferedList.C \
	Tracking.C 



#----------------------------------------------------------------------
# default: make library
#----------------------------------------------------------------------

default: $(TARGET_LIB)


#----------------------------------------------------------------------
# depend: list dependencies
#----------------------------------------------------------------------

depend:
	@ inclist `echo $(OBJS) | sed -e 's/[.]o/.C/g'`
	@ inclist `echo $(C_OBJS) | sed -e 's/[.]o/.c/g'`	

#----------------------------------------------------------------------
# clean: remove all object code and library
#----------------------------------------------------------------------

clean:
	- /bin/rm -f $(TARGET_LIB)
	- /bin/rm -f $(OBJS)
	(set incfiles=($(INCS)); cd $(INCDIR); /bin/rm -f $$incfiles)

#	- /bin/rm -f $(INCDIR)/*

#----------------------------------------------------------------------
# install: same as default
#----------------------------------------------------------------------

install: $(TARGET_LIB)

#----------------------------------------------------------------------
# rules
#----------------------------------------------------------------------

.PRECIOUS: %.C

$(TARGET_LIB):	$(OBJS) $(C_OBJS)
	- /bin/rm -f $(TARGET_LIB)
	ar -r $(TARGET_LIB) $(OBJS) $(C_OBJS)
	ranlib $(TARGET_LIB)
	- (set incfiles=($(INCS)); cd $(INCDIR); /bin/rm -f $$incfiles)
	cp $(INCS) $(INCDIR)

#	- (cd $(INCDIR); /bin/rm -f $(INCS))

%.C: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v

%.c: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.c,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v


#----------------------------------------------------------------------
# Object file dependencies
#----------------------------------------------------------------------

List.o: List.C List.h
BufferedList.o: BufferedList.C BufferedList.h List.h
AccurateGeom.o: AccurateGeom.C Spacecraft.h Meas.h InstrumentGeom.h AccurateGeom.h Qscat.h Misc.h Array.h BYUXTable.h Constants.h Matrix3.h Ephemeris.h Attitude.h Antenna.h LonLat.h CoastalMaps.h List.h EarthPosition.h CoordinateSwitch.h Tracking.h Scatterometer.h CheckFrame.h Topo.h BufferedList.h Beam.h LandMap.h Wind.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h Index.h
AmbigTable.o: AmbigTable.C AmbigTable.h Array.h
Antenna.o: Antenna.C Antenna.h Constants.h Beam.h CoordinateSwitch.h Matrix3.h Attitude.h
Array.o: Array.C Array.h
AngleInterval.o: AngleInterval.C AngleInterval.h List.h Array.h Misc.h Wind.h Constants.h Index.h Matrix.h Distributions.h LonLatWind.h WindVector.h LonLat.h EarthPosition.h Matrix3.h
Attitude.o: Attitude.C Attitude.h Constants.h
BaseFile.o: BaseFile.C BaseFile.h
Beam.o: Beam.C Beam.h Array.h Constants.h Misc.h CoordinateSwitch.h Matrix3.h Attitude.h
BYUXTable.o: BYUXTable.C BYUXTable.h Qscat.h Sigma0.h InstrumentGeom.h CheckFrame.h Topo.h Spacecraft.h Meas.h Array.h Antenna.h Tracking.h Scatterometer.h CoordinateSwitch.h Distributions.h EarthPosition.h Ephemeris.h LonLat.h Matrix3.h Attitude.h Wind.h Constants.h CoastalMaps.h List.h Beam.h BufferedList.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h Misc.h LandMap.h
CheckFrame.o: CheckFrame.C CheckFrame.h Constants.h Attitude.h Matrix3.h EarthPosition.h Wind.h Meas.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h LonLat.h Index.h List.h Misc.h Antenna.h Ephemeris.h CoastalMaps.h Beam.h BufferedList.h LandMap.h CoordinateSwitch.h
CoastalMaps.o: CoastalMaps.C CoastalMaps.h Meas.h Constants.h Array.h L2A.h LandMap.h CoordinateSwitch.h Antenna.h LonLat.h Ephemeris.h Attitude.h List.h BaseFile.h Matrix3.h Beam.h EarthPosition.h BufferedList.h
ColorMap.o: ColorMap.C ColorMap.h Array.h List.h
ConfigList.o: ConfigList.C ConfigList.h List.h Options.h
ConfigSim.o: ConfigSim.C ConfigSim.h ConfigSimDefs.h OvwmConfigDefs.h SpacecraftSim.h XTable.h BYUXTable.h Misc.h L00.h L1A.h L1AToL1B.h L1B.h L1BHdf.h L2A.h L2B.h L2AToL2B.h Constants.h Distributions.h Tracking.h Kpm.h EarthField.h Kp.h GenericGeom.h ETime.h Wind.h Grid.h ConfigList.h FbbTable.h Rain.h YahyaAntenna.h Spacecraft.h Qscat.h Meas.h Array.h CheckFrame.h Topo.h BaseFile.h L00Frame.h L1AFrame.h L1AGSFrame.h Ephemeris.h LandMap.h WindSwath.h GMF.h Index.h LonLat.h Kpr.h EarthPosition.h CoordinateSwitch.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h List.h Options.h Matrix3.h Attitude.h Antenna.h Scatterometer.h CoastalMaps.h BufferedList.h MiscTable.h Beam.h
CoordinateSwitch.o: CoordinateSwitch.C CoordinateSwitch.h Matrix3.h Attitude.h
Distributions.o: Distributions.C Distributions.h Constants.h
EarthField.o: EarthField.C LonLat.h Constants.h Array.h Index.h EarthField.h List.h EarthPosition.h Matrix3.h
EarthGeom.o: EarthGeom.C EarthGeom.h Vect.h Transform.h Constants.h Mat.h
EarthPosition.o: EarthPosition.C EarthPosition.h Constants.h LonLat.h CoordinateSwitch.h Matrix3.h List.h Attitude.h
Ephemeris.o: Ephemeris.C Ephemeris.h List.h List.C Interpolate.h Constants.h Matrix3.h BufferedList.h EarthPosition.h
ETime.o: ETime.C ETime.h
FbbTable.o: FbbTable.C FbbTable.h Qscat.h Sigma0.h InstrumentGeom.h CheckFrame.h BYUXTable.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h CoordinateSwitch.h Distributions.h EarthPosition.h Ephemeris.h LonLat.h Matrix3.h Attitude.h Wind.h Array.h Topo.h Beam.h Constants.h CoastalMaps.h List.h BufferedList.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h Misc.h LandMap.h
Flower.o: Flower.C Flower.h GMF.h L2AH.h MiscTable.h Wind.h Meas.h Constants.h Kp.h L2A.h ETime.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h LonLat.h Index.h List.h Misc.h Antenna.h Ephemeris.h Attitude.h CoastalMaps.h Kpm.h Kpr.h BaseFile.h EarthPosition.h Beam.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h EarthField.h L00Frame.h
GMF.o: GMF.C GMF.h GSparameters.h Meas.h Misc.h Interpolate.h Constants.h Beam.h List.h AngleInterval.h Array.h MiscTable.h Wind.h Kp.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h CoordinateSwitch.h Index.h Matrix.h Distributions.h LonLatWind.h WindVector.h Kpm.h Kpr.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h EarthField.h L00Frame.h
GenericGeom.o: GenericGeom.C GenericGeom.h CoordinateSwitch.h Constants.h EarthPosition.h Matrix3.h Attitude.h
GeomNoiseFile.o: GeomNoiseFile.C GeomNoiseFile.h Constants.h
Grid.o: Grid.C Grid.h Meas.h Sigma0.h Misc.h Array.h L1B.h L2A.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h CoordinateSwitch.h Spacecraft.h Distributions.h Qscat.h BaseFile.h Beam.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h Constants.h Tracking.h Scatterometer.h
HdfHelp.o: HdfHelp.C HdfHelp.h
hdf_support.o: hdf_support.C hdf_support.h
Image.o: Image.C Image.h Array.h Misc.h Constants.h
Index.o: Index.C Index.h
InstrumentGeom.o: InstrumentGeom.C Ephemeris.h Attitude.h Antenna.h GenericGeom.h Spacecraft.h Meas.h InstrumentGeom.h Misc.h Qscat.h Interpolate.h Array.h BYUXTable.h Matrix3.h BufferedList.h EarthPosition.h Beam.h CoordinateSwitch.h Constants.h LonLat.h CoastalMaps.h List.h Tracking.h Scatterometer.h CheckFrame.h Topo.h LandMap.h Wind.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h Index.h
Interpolate.o: Interpolate.C Interpolate.h
Jpeg.o: Jpeg.C Jpeg.h ColorMap.h List.h
Kp.o: Kp.C Kp.h Kpm.h Kpr.h Index.h EarthField.h Distributions.h Meas.h L00Frame.h LonLat.h Antenna.h Ephemeris.h Attitude.h CoastalMaps.h List.h EarthPosition.h Beam.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h
KpStatistics.o: KpStatistics.C KpStatistics.h Array.h Index.h GMF.h Wind.h WindGrad.h Meas.h MiscTable.h Constants.h Kp.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h LonLat.h List.h Misc.h WindSwath.h Antenna.h Ephemeris.h Attitude.h CoastalMaps.h Kpm.h Kpr.h EarthPosition.h Beam.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h EarthField.h L00Frame.h
Kpm.o: Kpm.C Kpm.h Constants.h Array.h Index.h EarthField.h Distributions.h Meas.h LonLat.h Antenna.h Ephemeris.h Attitude.h CoastalMaps.h List.h EarthPosition.h Beam.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h
Kpr.o: Kpr.C Kpr.h Constants.h Misc.h Array.h GenericGeom.h CoordinateSwitch.h Meas.h L00Frame.h EarthPosition.h Matrix3.h Attitude.h Antenna.h LonLat.h Ephemeris.h CoastalMaps.h List.h Beam.h BufferedList.h LandMap.h
LandMap.o: LandMap.C Array.h Constants.h LandMap.h Misc.h LonLat.h List.h EarthPosition.h Matrix3.h
L00.o: L00.C L00.h BaseFile.h L00Frame.h Attitude.h
L00Frame.o: L00Frame.C L00Frame.h Attitude.h
L00ToL1A.o: L00ToL1A.C L00ToL1A.h L00.h L1A.h BaseFile.h L00Frame.h L1AFrame.h L1AGSFrame.h Attitude.h
L1A.o: L1A.C L1A.h Misc.h BaseFile.h L1AFrame.h L1AGSFrame.h Attitude.h
L1AH.o: L1AH.C L1AH.h ETime.h Sds.h Qscat.h L1A.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h BaseFile.h L1AFrame.h L1AGSFrame.h Beam.h Constants.h Matrix3.h Ephemeris.h Attitude.h CoordinateSwitch.h EarthPosition.h LonLat.h CoastalMaps.h List.h BufferedList.h LandMap.h
L1AFrame.o: L1AFrame.C Constants.h EarthPosition.h L1AHdf.h L1AFrame.h Matrix3.h L1A.h Attitude.h L1AGSFrame.h BaseFile.h
L1AGSFrame.o: L1AGSFrame.C L1AGSFrame.h
L1AHdf.o: L1AHdf.C L1AHdf.h L1A.h BaseFile.h L1AFrame.h L1AGSFrame.h Attitude.h
L1AToL1B.o: L1AToL1B.C L1AToL1B.h CheckFrame.h InstrumentGeom.h Sigma0.h Qscat.h L1A.h Spacecraft.h Ephemeris.h L1B.h XTable.h BYUXTable.h LandMap.h Attitude.h Matrix3.h EarthPosition.h Wind.h Meas.h CoordinateSwitch.h LonLat.h Distributions.h Antenna.h Tracking.h Scatterometer.h BaseFile.h L1AFrame.h L1AGSFrame.h Constants.h BufferedList.h Array.h Topo.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h List.h Misc.h CoastalMaps.h Beam.h
L1B.o: L1B.C L1B.h Meas.h BaseFile.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Beam.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h
L1BHdf.o: L1BHdf.C L1BHdf.h L1B.h LandMap.h Meas.h BaseFile.h LonLat.h Antenna.h Ephemeris.h Attitude.h CoastalMaps.h List.h EarthPosition.h Beam.h Matrix3.h BufferedList.h CoordinateSwitch.h
L1BToL2A.o: L1BToL2A.C L1BToL2A.h Antenna.h Ephemeris.h InstrumentGeom.h GenericGeom.h L1B.h Grid.h Beam.h Matrix3.h BufferedList.h EarthPosition.h CoordinateSwitch.h Spacecraft.h LonLat.h Meas.h Qscat.h BaseFile.h Array.h L2A.h List.h Attitude.h Constants.h CoastalMaps.h Tracking.h Scatterometer.h LandMap.h
L2A.o: L2A.C L2A.h Constants.h Qscat.h Misc.h BaseFile.h Meas.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Beam.h Matrix3.h CoordinateSwitch.h EarthPosition.h BufferedList.h LandMap.h
L2AH.o: L2AH.C L2AH.h Sds.h Constants.h Misc.h L2A.h ETime.h BaseFile.h Meas.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Beam.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h
L2AHdf.o: L2AHdf.C L2AHdf.h Constants.h L2A.h BaseFile.h Meas.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Beam.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h
L2AToL2B.o: L2AToL2B.C L2AToL2B.h Constants.h Misc.h L2A.h L2B.h GMF.h BaseFile.h Meas.h WindSwath.h MiscTable.h Wind.h Kp.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h Index.h Kpm.h Kpr.h Beam.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h EarthField.h L00Frame.h
L2B.o: L2B.C L2B.h WindVector.h Wind.h Misc.h HdfHelp.h Sds.h BaseFile.h WindSwath.h Constants.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h LonLat.h Index.h List.h EarthPosition.h Matrix3.h
LonLat.o: LonLat.C LonLat.h Constants.h EarthPosition.h List.h Matrix3.h
Malloc1.o: Malloc1.C Malloc1.h
Mat.o: Mat.C Mat.h Vect.h
Matrix.o: Matrix.C Matrix.h Misc.h
Matrix3.o: Matrix3.C Matrix3.h Constants.h
Meas.o: Meas.C L1BHdf.h L2AHdf.h Meas.h InstrumentGeom.h Beam.h Constants.h LonLat.h GenericGeom.h ETime.h Array.h L1B.h LandMap.h L2A.h Antenna.h Ephemeris.h Attitude.h CoastalMaps.h List.h EarthPosition.h CoordinateSwitch.h Spacecraft.h Matrix3.h Qscat.h BaseFile.h BufferedList.h Tracking.h Scatterometer.h
Metrics.o: Metrics.C Metrics.h Wind.h LonLatWind.h WindSwath.h Constants.h Matrix.h Distributions.h AngleInterval.h WindVector.h LonLat.h Index.h List.h Misc.h EarthPosition.h Matrix3.h
Misc.o: Misc.C Misc.h Constants.h Matrix.h Array.h
MiscTable.o: MiscTable.C MiscTable.h Interpolate.h Constants.h Array.h Meas.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Beam.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h
MLP.o: MLP.C MLP.h Array.h Distributions.h MLPData.h
MLPData.o: MLPData.C Array.h MLPData.h MLP.h
PMeas.o: PMeas.C Pscat.h Distributions.h Qscat.h PMeas.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h Beam.h Constants.h Matrix3.h Ephemeris.h Attitude.h CoordinateSwitch.h EarthPosition.h LonLat.h CoastalMaps.h List.h BufferedList.h LandMap.h
PointList.o: PointList.C PointList.h List.h
PointTargetResponseTable.o: PointTargetResponseTable.C PointTargetResponseTable.h Constants.h
Pscat.o: Pscat.C Pscat.h PMeas.h Misc.h Distributions.h Qscat.h Meas.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Beam.h Constants.h Matrix3.h CoordinateSwitch.h EarthPosition.h BufferedList.h LandMap.h
PscatConfig.o: PscatConfig.C PscatConfig.h QscatConfig.h ConfigSimDefs.h PscatConfigDefs.h ConfigSim.h Pscat.h PscatSim.h PscatL1A.h PscatL1AToL1B.h ConfigList.h Qscat.h QscatSim.h SpacecraftSim.h XTable.h L00.h L1A.h L1AToL1B.h L1B.h L2A.h L2B.h L2AToL2B.h Wind.h Grid.h Tracking.h Kp.h FbbTable.h Rain.h YahyaAntenna.h PMeas.h LandMap.h Distributions.h BYUXTable.h GMF.h Meas.h BaseFile.h Attitude.h Spacecraft.h Ephemeris.h List.h Options.h Antenna.h Scatterometer.h CheckFrame.h L1AFrame.h Sigma0Map.h L00Frame.h L1AGSFrame.h WindSwath.h Constants.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h LonLat.h Index.h Misc.h Array.h Kpm.h Kpr.h CoordinateSwitch.h Topo.h MiscTable.h CoastalMaps.h Matrix3.h BufferedList.h EarthPosition.h Beam.h EarthField.h
PscatL1A.o: PscatL1A.C Pscat.h PscatL1A.h Constants.h Misc.h Qscat.h PMeas.h BaseFile.h Attitude.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h Beam.h Matrix3.h Ephemeris.h CoordinateSwitch.h EarthPosition.h LonLat.h CoastalMaps.h List.h BufferedList.h LandMap.h
PscatL1AToL1B.o: PscatL1AToL1B.C PscatL1AToL1B.h CheckFrame.h InstrumentGeom.h Sigma0.h Pscat.h PscatL1A.h Spacecraft.h Ephemeris.h L1B.h XTable.h BYUXTable.h LandMap.h Attitude.h Matrix3.h EarthPosition.h Wind.h Meas.h CoordinateSwitch.h LonLat.h Qscat.h Distributions.h PMeas.h BaseFile.h Constants.h BufferedList.h Array.h Topo.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h List.h Misc.h Antenna.h CoastalMaps.h Tracking.h Scatterometer.h Beam.h
PscatSim.o: PscatSim.C PscatSim.h PMeas.h CheckFrame.h InstrumentGeom.h Sigma0.h AccurateGeom.h Beam.h Pscat.h LandMap.h Distributions.h XTable.h BYUXTable.h Wind.h GMF.h Meas.h PscatL1A.h QscatSim.h Attitude.h Matrix3.h EarthPosition.h CoordinateSwitch.h Ephemeris.h Spacecraft.h LonLat.h Qscat.h Array.h Topo.h Constants.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h List.h Misc.h MiscTable.h Kp.h Antenna.h CoastalMaps.h BaseFile.h L1AFrame.h Sigma0Map.h BufferedList.h Tracking.h Scatterometer.h Kpm.h Kpr.h L1AGSFrame.h EarthField.h L00Frame.h
Pulser.o: Pulser.C Pulser.h Misc.h List.h Constants.h ConfigList.h Options.h
Qscat.o: Qscat.C Qscat.h Constants.h InstrumentGeom.h GenericGeom.h Interpolate.h Misc.h BYUXTable.h AccurateGeom.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h EarthPosition.h CoordinateSwitch.h Ephemeris.h LonLat.h Matrix3.h Array.h CheckFrame.h Topo.h Beam.h Attitude.h CoastalMaps.h List.h BufferedList.h Wind.h LandMap.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h Index.h
QscatConfig.o: QscatConfig.C Qscat.h ConfigList.h QscatConfig.h QscatConfigDefs.h Misc.h QscatSim.h ConfigSimDefs.h Constants.h ConfigSim.h Interpolate.h ETime.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h List.h Options.h LandMap.h Distributions.h XTable.h BYUXTable.h Wind.h GMF.h CheckFrame.h L1AFrame.h Sigma0Map.h SpacecraftSim.h L00.h L1A.h L1AToL1B.h L1B.h L2A.h L2B.h L2AToL2B.h Grid.h Kp.h FbbTable.h Rain.h YahyaAntenna.h Beam.h Matrix3.h Ephemeris.h Attitude.h CoordinateSwitch.h EarthPosition.h LonLat.h CoastalMaps.h Array.h Topo.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h MiscTable.h L1AGSFrame.h BaseFile.h L00Frame.h WindSwath.h Kpm.h Kpr.h BufferedList.h EarthField.h
QscatSim.o: QscatSim.C QscatSim.h CheckFrame.h InstrumentGeom.h Sigma0.h AccurateGeom.h Beam.h Sigma0Map.h Qscat.h LandMap.h Distributions.h XTable.h BYUXTable.h Wind.h GMF.h Meas.h L1AFrame.h Attitude.h Matrix3.h EarthPosition.h CoordinateSwitch.h Ephemeris.h Spacecraft.h LonLat.h Antenna.h Tracking.h Scatterometer.h Array.h Topo.h Constants.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h List.h Misc.h MiscTable.h Kp.h CoastalMaps.h L1AGSFrame.h BufferedList.h Kpm.h Kpr.h EarthField.h L00Frame.h
Ovwm.o: Ovwm.C Ovwm.h Constants.h InstrumentGeom.h GenericGeom.h Interpolate.h Misc.h BYUXTable.h AccurateGeom.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h AmbigTable.h PointTargetResponseTable.h EarthPosition.h CoordinateSwitch.h Ephemeris.h LonLat.h Matrix3.h Qscat.h Array.h CheckFrame.h Topo.h Beam.h Attitude.h CoastalMaps.h List.h BufferedList.h Wind.h LandMap.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h Index.h
OvwmL1A.o: OvwmL1A.C OvwmL1A.h Misc.h Constants.h BaseFile.h Attitude.h
OvwmL1AToL1B.o: OvwmL1AToL1B.C OvwmL1AToL1B.h CheckFrame.h InstrumentGeom.h OvwmSigma0.h Ovwm.h Array.h OvwmL1A.h Spacecraft.h Ephemeris.h L1B.h SchToXyz.h Utils.h LandMap.h Topo.h PointTargetResponseTable.h AmbigTable.h Attitude.h Matrix3.h EarthPosition.h Wind.h Meas.h CoordinateSwitch.h LonLat.h Qscat.h Distributions.h Antenna.h Tracking.h Scatterometer.h Constants.h BaseFile.h BufferedList.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h List.h Misc.h CoastalMaps.h Beam.h
OvwmConfig.o: OvwmConfig.C Ovwm.h ConfigList.h OvwmConfig.h OvwmConfigDefs.h Misc.h OvwmSim.h ConfigSimDefs.h Constants.h ConfigSim.h Interpolate.h ETime.h Antenna.h Tracking.h Spacecraft.h Scatterometer.h Meas.h AmbigTable.h PointTargetResponseTable.h List.h Options.h OvwmL1A.h OvwmL1AToL1B.h LandMap.h Distributions.h XTable.h BYUXTable.h Wind.h GMF.h CheckFrame.h Sigma0Map.h L1B.h SchToXyz.h Utils.h Rain.h SpacecraftSim.h L00.h L1A.h L1AToL1B.h L2A.h L2B.h L2AToL2B.h Grid.h Kp.h FbbTable.h YahyaAntenna.h Beam.h Matrix3.h Ephemeris.h Attitude.h CoordinateSwitch.h EarthPosition.h LonLat.h CoastalMaps.h BaseFile.h Array.h Topo.h Qscat.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h MiscTable.h L00Frame.h L1AFrame.h L1AGSFrame.h WindSwath.h Kpm.h Kpr.h BufferedList.h EarthField.h
OvwmSim.o: OvwmSim.C OvwmSim.h CheckFrame.h InstrumentGeom.h OvwmSigma0.h Sigma0.h AccurateGeom.h Beam.h Sigma0Map.h Ovwm.h LandMap.h Distributions.h XTable.h BYUXTable.h Wind.h GMF.h Meas.h OvwmL1A.h L1B.h SchToXyz.h Utils.h AmbigTable.h Rain.h Attitude.h Matrix3.h EarthPosition.h CoordinateSwitch.h Ephemeris.h Spacecraft.h LonLat.h Qscat.h Antenna.h Tracking.h Scatterometer.h PointTargetResponseTable.h Array.h Topo.h Constants.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h Index.h List.h Misc.h MiscTable.h Kp.h CoastalMaps.h BaseFile.h BufferedList.h Kpm.h Kpr.h EarthField.h L00Frame.h
OvwmSigma0.o: OvwmSigma0.C CoordinateSwitch.h Spacecraft.h Meas.h OvwmSigma0.h Ovwm.h Distributions.h Misc.h Matrix3.h Attitude.h Constants.h Ephemeris.h Antenna.h LonLat.h CoastalMaps.h List.h Tracking.h Scatterometer.h AmbigTable.h PointTargetResponseTable.h BufferedList.h EarthPosition.h Beam.h LandMap.h
Quat.o: Quat.C Quat.h Vect.h Mat.h
Rain.o: Rain.C Rain.h Array.h Constants.h Matrix.h LonLat.h Index.h List.h Meas.h Spacecraft.h CoordinateSwitch.h EarthPosition.h Antenna.h Ephemeris.h Attitude.h CoastalMaps.h Matrix3.h Beam.h BufferedList.h LandMap.h
RainDistribution.o: RainDistribution.C RainDistribution.h Meas.h Antenna.h LonLat.h Ephemeris.h Attitude.h CoastalMaps.h List.h Beam.h EarthPosition.h Matrix3.h BufferedList.h LandMap.h CoordinateSwitch.h
RainHeightTable.o: RainHeightTable.C Array.h RainHeightTable.h Constants.h Index.h
Scatterometer.o: Scatterometer.C Scatterometer.h InstrumentGeom.h GenericGeom.h CoordinateSwitch.h EarthPosition.h Matrix3.h Spacecraft.h Meas.h Antenna.h Ephemeris.h LonLat.h Qscat.h Attitude.h Constants.h CoastalMaps.h List.h Beam.h BufferedList.h Tracking.h LandMap.h
SchToXyz.o: SchToXyz.C SchToXyz.h Utils.h Matrix3.h
Sds.o: Sds.C Sds.h
Sigma0.o: Sigma0.C CoordinateSwitch.h Spacecraft.h Meas.h Sigma0.h Qscat.h Distributions.h Misc.h Matrix3.h Attitude.h Constants.h Ephemeris.h Antenna.h LonLat.h CoastalMaps.h List.h Tracking.h Scatterometer.h BufferedList.h EarthPosition.h Beam.h LandMap.h
Sigma0Map.o: Sigma0Map.C Array.h Constants.h Sigma0Map.h LonLat.h List.h EarthPosition.h Matrix3.h
Spacecraft.o: Spacecraft.C Spacecraft.h Constants.h Matrix3.h Ephemeris.h Attitude.h BufferedList.h EarthPosition.h List.h
SpacecraftSim.o: SpacecraftSim.C Spacecraft.h SpacecraftSim.h Constants.h Matrix3.h Ephemeris.h Attitude.h Distributions.h BufferedList.h EarthPosition.h List.h
Sfile.o: Sfile.C Sfile.h List.h
Topo.o: Topo.C Topo.h Array.h Constants.h BYUXTable.h Spacecraft.h Qscat.h Meas.h CheckFrame.h Matrix3.h Ephemeris.h Attitude.h Antenna.h Tracking.h Scatterometer.h LonLat.h CoastalMaps.h List.h EarthPosition.h Wind.h BufferedList.h Beam.h CoordinateSwitch.h LandMap.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h Index.h Misc.h
Tracking.o: Tracking.C Tracking.h Constants.h Array.h
Transform.o: Transform.C Transform.h Vect.h Mat.h
Vect.o: Vect.C Vect.h Mat.h
Wind.o: Wind.C WindVector.h Wind.h GSparameters.h Array.h Constants.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h LonLat.h Index.h List.h Misc.h EarthPosition.h Matrix3.h
WindSwath.o: WindSwath.C WindSwath.h hdf_support.h Distributions.h Array.h Vect.h EarthGeom.h Wind.h Constants.h Matrix.h AngleInterval.h LonLatWind.h WindVector.h LonLat.h Index.h List.h Misc.h EarthPosition.h Matrix3.h
WindVector.o: WindVector.C WindVector.h
WindGrad.o: WindGrad.C WindGrad.h Wind.h WindSwath.h Array.h Constants.h Matrix.h Distributions.h AngleInterval.h LonLatWind.h WindVector.h LonLat.h Index.h List.h Misc.h EarthPosition.h Matrix3.h
XTable.o: XTable.C XTable.h Constants.h Misc.h Array.h GenericGeom.h CoordinateSwitch.h EarthPosition.h Matrix3.h Attitude.h
SeaPac.o: SeaPac.C SeaPac.h Constants.h
ENOFTable.o: ENOFTable.C Array.h ENOFTable.h Index.h
Options.o: Options.C Options.h
Utils.o: Utils.C Utils.h Constants.h Matrix3.h
dot_prod.o: dot_prod.c 
erfcomp_cheb.o: erfcomp_cheb.c function.h
qual_flag.o: qual_flag.c 
doub_factorial_double.o: doub_factorial_double.c function.h
factorial_double.o: factorial_double.c function.h
sphere_normal_prob.o: sphere_normal_prob.c function.h
doub_factorial_int.o: doub_factorial_int.c 
factorial_int.o: factorial_int.c 
stirling4.o: stirling4.c function.h
erf_cheb.o: erf_cheb.c function.h
int_rn_exp.o: int_rn_exp.c function.h
AscatL1bReader.o: AscatL1bReader.h




