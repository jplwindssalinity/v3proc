#======================================================================
# @(#) $Id$
# Makefile for all SIM object code
#======================================================================

SHELL =			/bin/csh

INCDIR =		../../include
TARGET_LIB =	../../lib/libSim.a
HDFINCDIR =     $(INCDIR)/hdf
DEBUGFLAGS =    -O3
INCFLAGS =      -I$(INCDIR) -I$(HDFINCDIR) -I.

CPPFLAGS =		$(DEBUGFLAGS) $(INCFLAGS) -fno-implicit-templates -Wall
SYSLIBS =		-lm
CCC =			gcc

CPINCFILES =    echo *.h

OBJS = \
	List.o \
	BufferedList.o \
	AccurateGeom.o \
	Antenna.o \
	Array.o \
	AngleInterval.o \
	Attitude.o \
	BaseFile.o \
	Beam.o \
	BYUXTable.o \
	CheckFrame.o \
	ConfigList.o \
	ConfigSim.o \
	CoordinateSwitch.o \
	Distributions.o \
	EarthField.o \
	EarthPosition.o \
	Ephemeris.o \
	GMF.o \
	GenericGeom.o \
	Grid.o \
	Index.o \
	InstrumentGeom.o \
	Interpolate.o \
	Kp.o \
	Kpm.o \
	Kpr.o \
	LandMap.o \
	L00.o \
	L00Frame.o \
	L00ToL1A.o \
	L1A.o \
	L1AFrame.o \
	L1AGSFrame.o \
	L1AToL1B.o \
	L1B.o \
	L1BFrame.o \
	L1BToL2A.o \
	L2A.o \
	L2AFrame.o \
	L2AToL2B.o \
	L2B.o \
	L2BFrame.o \
	LonLat.o \
	Malloc1.o \
	Matrix.o \
	Matrix3.o \
	Meas.o \
	Misc.o \
	PiscTable.o \
	Qscat.o \
	QscatConfig.o \
	QscatSim.o \
	Sigma0.o \
	Spacecraft.o \
	SpacecraftSim.o \
	Tracking.o \
	Wind.o \
	XTable.o 

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

$(TARGET_LIB):	$(OBJS)
	- /bin/rm -f $(TARGET_LIB)
	/usr/xpg4/bin/ar -r $(TARGET_LIB) $(OBJS)
	- (set incfiles=($(INCS)); cd $(INCDIR); /bin/rm -f $$incfiles)
	cp $(INCS) $(INCDIR)

#	- (cd $(INCDIR); /bin/rm -f $(INCS))

%.C: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v

#----------------------------------------------------------------------
# Object file dependencies
#----------------------------------------------------------------------

List.o: List.C List.h
BufferedList.o: BufferedList.C BufferedList.h List.h
AccurateGeom.o: AccurateGeom.C Spacecraft.h Meas.h InstrumentGeom.h AccurateGeom.h Qscat.h Misc.h Array.h Constants.h Matrix3.h Ephemeris.h Attitude.h Beam.h LonLat.h List.h EarthPosition.h CoordinateSwitch.h Antenna.h Tracking.h BufferedList.h
Antenna.o: Antenna.C Antenna.h Constants.h Beam.h CoordinateSwitch.h Matrix3.h Attitude.h
Array.o: Array.C Array.h
AngleInterval.o: AngleInterval.C AngleInterval.h List.h Array.h Misc.h Constants.h
Attitude.o: Attitude.C Attitude.h Constants.h
BaseFile.o: BaseFile.C BaseFile.h
Beam.o: Beam.C Beam.h Array.h Constants.h CoordinateSwitch.h Matrix3.h Attitude.h
BYUXTable.o: BYUXTable.C BYUXTable.h Qscat.h Sigma0.h InstrumentGeom.h CheckFrame.h Spacecraft.h Meas.h XTable.h Antenna.h Tracking.h CoordinateSwitch.h Distributions.h EarthPosition.h Ephemeris.h LonLat.h Matrix3.h Attitude.h Wind.h Constants.h Beam.h List.h BufferedList.h Misc.h Index.h Matrix.h
CheckFrame.o: CheckFrame.C CheckFrame.h Constants.h Attitude.h Matrix3.h EarthPosition.h Wind.h Misc.h List.h LonLat.h Index.h Matrix.h
ConfigList.o: ConfigList.C ConfigList.h List.h
ConfigSim.o: ConfigSim.C ConfigSim.h ConfigSimDefs.h SpacecraftSim.h XTable.h BYUXTable.h Misc.h L00.h L1A.h L1AToL1B.h L1B.h L2A.h L2B.h L2AToL2B.h Constants.h Distributions.h Tracking.h Kpm.h EarthField.h Kp.h Wind.h Grid.h ConfigList.h Spacecraft.h Qscat.h Meas.h CheckFrame.h BaseFile.h L00Frame.h L1AFrame.h L1AGSFrame.h Ephemeris.h LandMap.h L1BFrame.h L2AFrame.h L2BFrame.h GMF.h Index.h LonLat.h Kpr.h List.h Matrix.h Array.h Matrix3.h Attitude.h Antenna.h Beam.h EarthPosition.h BufferedList.h PiscTable.h CoordinateSwitch.h
CoordinateSwitch.o: CoordinateSwitch.C CoordinateSwitch.h Matrix3.h Attitude.h
Distributions.o: Distributions.C Distributions.h
EarthField.o: EarthField.C LonLat.h Constants.h Array.h Index.h EarthField.h List.h EarthPosition.h Matrix3.h
EarthPosition.o: EarthPosition.C EarthPosition.h Constants.h LonLat.h CoordinateSwitch.h Matrix3.h List.h Attitude.h
Ephemeris.o: Ephemeris.C Ephemeris.h List.h List.C Interpolate.h Constants.h Matrix3.h BufferedList.h EarthPosition.h
GMF.o: GMF.C GMF.h GSparameters.h Meas.h Interpolate.h Constants.h Beam.h List.h AngleInterval.h Array.h PiscTable.h Wind.h Kp.h LonLat.h Ephemeris.h Attitude.h CoordinateSwitch.h Misc.h Index.h Matrix.h Kpm.h Kpr.h EarthPosition.h Matrix3.h BufferedList.h EarthField.h Distributions.h L00Frame.h
GenericGeom.o: GenericGeom.C GenericGeom.h CoordinateSwitch.h Constants.h EarthPosition.h Matrix3.h Attitude.h
Grid.o: Grid.C Grid.h Meas.h Sigma0.h Array.h L1B.h L2A.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h Spacecraft.h Distributions.h Qscat.h BaseFile.h L1BFrame.h L2AFrame.h EarthPosition.h Matrix3.h BufferedList.h Constants.h Antenna.h Tracking.h
Index.o: Index.C Index.h
InstrumentGeom.o: InstrumentGeom.C Ephemeris.h Attitude.h Antenna.h GenericGeom.h Spacecraft.h Meas.h InstrumentGeom.h Misc.h Qscat.h Interpolate.h Array.h BYUXTable.h Matrix3.h BufferedList.h EarthPosition.h Beam.h CoordinateSwitch.h Constants.h LonLat.h List.h Tracking.h XTable.h CheckFrame.h Wind.h Index.h Matrix.h
Interpolate.o: Interpolate.C 
Kp.o: Kp.C Kp.h Kpm.h Kpr.h Index.h EarthField.h Distributions.h Meas.h L00Frame.h LonLat.h Beam.h Ephemeris.h Attitude.h List.h EarthPosition.h CoordinateSwitch.h Matrix3.h BufferedList.h
Kpm.o: Kpm.C Kpm.h Constants.h Array.h Index.h EarthField.h Distributions.h LonLat.h List.h EarthPosition.h Matrix3.h
Kpr.o: Kpr.C Kpr.h Constants.h Misc.h Array.h GenericGeom.h CoordinateSwitch.h Meas.h L00Frame.h EarthPosition.h Matrix3.h Attitude.h Beam.h LonLat.h Ephemeris.h List.h BufferedList.h
LandMap.o: LandMap.C Array.h Constants.h LandMap.h LonLat.h List.h EarthPosition.h Matrix3.h
L00.o: L00.C L00.h BaseFile.h L00Frame.h Attitude.h
L00Frame.o: L00Frame.C L00Frame.h Attitude.h
L00ToL1A.o: L00ToL1A.C L00ToL1A.h L00.h L1A.h BaseFile.h L00Frame.h L1AFrame.h L1AGSFrame.h Attitude.h
L1A.o: L1A.C L1A.h BaseFile.h L1AFrame.h L1AGSFrame.h Attitude.h
L1AFrame.o: L1AFrame.C L1AFrame.h Constants.h Attitude.h L1AGSFrame.h
L1AGSFrame.o: L1AGSFrame.C L1AGSFrame.h
L1AToL1B.o: L1AToL1B.C L1AToL1B.h CheckFrame.h InstrumentGeom.h Sigma0.h L1A.h Spacecraft.h Ephemeris.h L1B.h XTable.h BYUXTable.h LandMap.h Attitude.h Matrix3.h EarthPosition.h Wind.h CoordinateSwitch.h LonLat.h Meas.h Qscat.h Distributions.h BaseFile.h L1AFrame.h L1AGSFrame.h Constants.h BufferedList.h L1BFrame.h Misc.h List.h Index.h Matrix.h Beam.h Antenna.h Tracking.h
L1B.o: L1B.C L1B.h BaseFile.h L1BFrame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L1BFrame.o: L1BFrame.C L1BFrame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L1BToL2A.o: L1BToL2A.C L1BToL2A.h Antenna.h Ephemeris.h InstrumentGeom.h GenericGeom.h L1B.h Grid.h Beam.h Matrix3.h BufferedList.h EarthPosition.h CoordinateSwitch.h Spacecraft.h LonLat.h Meas.h Qscat.h BaseFile.h L1BFrame.h Array.h L2A.h List.h Attitude.h Constants.h Tracking.h L2AFrame.h
L2A.o: L2A.C L2A.h BaseFile.h L2AFrame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L2AFrame.o: L2AFrame.C L2AFrame.h Constants.h Misc.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L2AToL2B.o: L2AToL2B.C L2AToL2B.h Constants.h Misc.h L2A.h L2B.h GMF.h BaseFile.h L2AFrame.h L2BFrame.h PiscTable.h Wind.h Meas.h Kp.h Beam.h List.h LonLat.h Index.h Matrix.h Ephemeris.h Attitude.h Kpm.h Kpr.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h EarthField.h Distributions.h L00Frame.h
L2B.o: L2B.C L2B.h BaseFile.h L2BFrame.h Wind.h Misc.h List.h LonLat.h Index.h Matrix.h EarthPosition.h Matrix3.h
L2BFrame.o: L2BFrame.C L2BFrame.h Wind.h Misc.h List.h LonLat.h Index.h Matrix.h EarthPosition.h Matrix3.h
LonLat.o: LonLat.C LonLat.h Constants.h EarthPosition.h List.h Matrix3.h
Malloc1.o: Malloc1.C Malloc1.h
Matrix.o: Matrix.C Matrix.h Misc.h
Matrix3.o: Matrix3.C Matrix3.h Constants.h
Meas.o: Meas.C Meas.h Beam.h Constants.h LonLat.h GenericGeom.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
Misc.o: Misc.C Misc.h Constants.h
PiscTable.o: PiscTable.C PiscTable.h Interpolate.h Constants.h Array.h Beam.h CoordinateSwitch.h Matrix3.h Attitude.h
Qscat.o: Qscat.C Qscat.h Constants.h InstrumentGeom.h Antenna.h Tracking.h Spacecraft.h EarthPosition.h CoordinateSwitch.h Ephemeris.h LonLat.h Matrix3.h Meas.h Beam.h Attitude.h BufferedList.h List.h
QscatConfig.o: QscatConfig.C Qscat.h ConfigList.h QscatConfig.h QscatConfigDefs.h Misc.h QscatSim.h ConfigSimDefs.h Constants.h ConfigSim.h Interpolate.h Antenna.h Tracking.h Spacecraft.h List.h LandMap.h Distributions.h XTable.h BYUXTable.h Wind.h GMF.h Meas.h CheckFrame.h L1AFrame.h SpacecraftSim.h L00.h L1A.h L1AToL1B.h L1B.h L2A.h L2B.h L2AToL2B.h Grid.h Kp.h Beam.h Matrix3.h Ephemeris.h Attitude.h LonLat.h Index.h Matrix.h PiscTable.h EarthPosition.h L1AGSFrame.h BaseFile.h L00Frame.h L1BFrame.h L2AFrame.h L2BFrame.h Array.h Kpm.h Kpr.h CoordinateSwitch.h BufferedList.h EarthField.h
QscatSim.o: QscatSim.C QscatSim.h CheckFrame.h InstrumentGeom.h Sigma0.h AccurateGeom.h Beam.h Qscat.h LandMap.h Distributions.h XTable.h BYUXTable.h Wind.h GMF.h Meas.h L1AFrame.h Attitude.h Matrix3.h EarthPosition.h CoordinateSwitch.h Ephemeris.h Spacecraft.h LonLat.h Antenna.h Tracking.h Misc.h List.h Index.h Matrix.h PiscTable.h Constants.h Kp.h L1AGSFrame.h BufferedList.h Kpm.h Kpr.h EarthField.h L00Frame.h
Sigma0.o: Sigma0.C CoordinateSwitch.h Spacecraft.h Meas.h Sigma0.h Qscat.h Distributions.h Matrix3.h Attitude.h Constants.h Ephemeris.h Beam.h LonLat.h List.h Antenna.h Tracking.h BufferedList.h EarthPosition.h
Spacecraft.o: Spacecraft.C Spacecraft.h Constants.h Matrix3.h Ephemeris.h Attitude.h BufferedList.h EarthPosition.h List.h
SpacecraftSim.o: SpacecraftSim.C Spacecraft.h SpacecraftSim.h Constants.h Matrix3.h Ephemeris.h Attitude.h Distributions.h BufferedList.h EarthPosition.h List.h
Tracking.o: Tracking.C Tracking.h Constants.h Array.h
Wind.o: Wind.C Wind.h Constants.h GSparameters.h Misc.h Array.h LonLat.h List.h Index.h Matrix.h EarthPosition.h Matrix3.h
XTable.o: XTable.C XTable.h Constants.h Misc.h Array.h GenericGeom.h CoordinateSwitch.h EarthPosition.h Matrix3.h Attitude.h


