#======================================================================
# @(#) $Id$
# Makefile for all SIM object code
#======================================================================

SHELL =			/bin/csh

INCDIR =		../../include
TARGET_LIB =	../../lib/libSim.a
DEBUGFLAGS = -O3



CPPFLAGS =		$(DEBUGFLAGS) -fno-implicit-templates -Wall
SYSLIBS =		-lm
CCC =			gcc

OBJS = \
	List.o \
	BufferedList.o \
	AccurateGeom.o \
	Antenna.o \
	AntennaSim.o \
	Array.o \
	Attitude.o \
	BaseFile.o \
	Beam.o \
	BeamSim.o \
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
	Instrument.o \
	InstrumentSim.o \
	InstrumentSimAccurate.o \
	InstrumentGeom.o \
	Interpolate.o \
	Kp.o \
	Kpm.o \
	Kpr.o \
	L00.o \
	L00Frame.o \
	L00ToL1A.o \
	L1A.o \
	L1AFrame.o \
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
	Matrix3.o \
	Meas.o \
	Misc.o \
	PiscTable.o \
	Sigma0.o \
	Spacecraft.o \
	SpacecraftSim.o \
	Tracking.o \
	Wind.o \
        XTable.o 

INCS = \
	*.h \
	List.C \
	BufferedList.C


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
	- /bin/rm -f $(INCDIR)/*

#----------------------------------------------------------------------
# install: same as default
#----------------------------------------------------------------------

install: $(TARGET_LIB)

#----------------------------------------------------------------------
# rules
#----------------------------------------------------------------------

$(TARGET_LIB):	$(OBJS)
	- /bin/rm -f $(TARGET_LIB)
	/usr/xpg4/bin/ar -r $(TARGET_LIB) $(OBJS)
	- (cd $(INCDIR); /bin/rm -f $(INCS))
	cp $(INCS) $(INCDIR)

%.C: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v

#----------------------------------------------------------------------
# Object file dependancies
#----------------------------------------------------------------------

AccurateGeom.o: AccurateGeom.C CoordinateSwitch.h Ephemeris.h Attitude.h Antenna.h Matrix3.h GenericGeom.h Spacecraft.h Instrument.h LonLat.h InstrumentGeom.h AccurateGeom.h Interpolate.h Array.h Misc.h InstrumentSim.h BufferedList.h EarthPosition.h Beam.h Constants.h InstConsts.h List.h Meas.h Wind.h GMF.h AntennaSim.h L00.h Distributions.h XTable.h Kpm.h Tracking.h Index.h PiscTable.h Kp.h BaseFile.h L00Frame.h EarthField.h Kpr.h
Antenna.o: Antenna.C Antenna.h Beam.h Constants.h CoordinateSwitch.h Attitude.h Tracking.h Matrix3.h
AntennaSim.o: AntennaSim.C AntennaSim.h Constants.h Antenna.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h Tracking.h
Array.o: Array.C Array.h
Attitude.o: Attitude.C Attitude.h
BaseFile.o: BaseFile.C BaseFile.h
Beam.o: Beam.C Beam.h Array.h Constants.h Attitude.h CoordinateSwitch.h Tracking.h Matrix3.h
BeamSim.o: BeamSim.C BeamSim.h
BufferedList.o: BufferedList.C BufferedList.h List.h
ConfigList.o: ConfigList.C ConfigList.h List.h
ConfigSim.o: ConfigSim.C ConfigSim.h ConfigSimDefs.h InstrumentSim.h InstrumentSimAccurate.h SpacecraftSim.h XTable.h Misc.h L00.h L1A.h L1AToL1B.h L1B.h L2A.h L2B.h Constants.h Distributions.h Tracking.h Kpm.h EarthField.h Kp.h Wind.h Grid.h ConfigList.h Antenna.h Instrument.h GMF.h AntennaSim.h Spacecraft.h BaseFile.h L00Frame.h L1AFrame.h Ephemeris.h L1BFrame.h L2AFrame.h L2BFrame.h Index.h LonLat.h Kpr.h List.h Array.h Meas.h CoordinateSwitch.h Beam.h Attitude.h InstConsts.h PiscTable.h Matrix3.h BufferedList.h EarthPosition.h
CoordinateSwitch.o: CoordinateSwitch.C CoordinateSwitch.h Matrix3.h Attitude.h
Distributions.o: Distributions.C Distributions.h
EarthField.o: EarthField.C LonLat.h Constants.h Array.h EarthField.h List.h EarthPosition.h Matrix3.h Index.h
EarthPosition.o: EarthPosition.C EarthPosition.h Constants.h LonLat.h CoordinateSwitch.h Matrix3.h List.h Attitude.h
Ephemeris.o: Ephemeris.C Ephemeris.h List.h List.C Interpolate.h Constants.h Matrix3.h BufferedList.h EarthPosition.h
GMF.o: GMF.C GMF.h Meas.h Interpolate.h Constants.h Beam.h PiscTable.h Wind.h Kp.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h Tracking.h Misc.h Index.h Kpm.h Kpr.h EarthPosition.h Matrix3.h BufferedList.h EarthField.h Distributions.h L00Frame.h
GenericGeom.o: GenericGeom.C GenericGeom.h CoordinateSwitch.h Constants.h EarthPosition.h Matrix3.h Attitude.h
Grid.o: Grid.C Grid.h Meas.h Sigma0.h Array.h L1B.h L2A.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h Spacecraft.h Instrument.h Wind.h BaseFile.h L1BFrame.h L2AFrame.h CoordinateSwitch.h Tracking.h EarthPosition.h Matrix3.h BufferedList.h Constants.h Antenna.h InstConsts.h Misc.h Index.h
Index.o: Index.C Index.h
Instrument.o: Instrument.C Instrument.h Antenna.h InstConsts.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h Tracking.h Constants.h
InstrumentGeom.o: InstrumentGeom.C CoordinateSwitch.h Ephemeris.h Attitude.h Antenna.h Matrix3.h GenericGeom.h Spacecraft.h Instrument.h LonLat.h InstrumentGeom.h Interpolate.h Array.h Misc.h Constants.h BufferedList.h EarthPosition.h Beam.h InstConsts.h List.h Meas.h Tracking.h
InstrumentSim.o: InstrumentSim.C InstrumentSim.h Instrument.h GenericGeom.h InstrumentGeom.h Ephemeris.h Sigma0.h Constants.h Kpm.h Antenna.h Wind.h GMF.h AntennaSim.h L00.h Spacecraft.h Distributions.h XTable.h InstConsts.h EarthPosition.h CoordinateSwitch.h LonLat.h Matrix3.h Meas.h BufferedList.h Index.h EarthField.h Beam.h Attitude.h Misc.h List.h PiscTable.h Kp.h BaseFile.h L00Frame.h Tracking.h Kpr.h
InstrumentSimAccurate.o: InstrumentSimAccurate.C InstrumentSim.h Instrument.h GenericGeom.h InstrumentGeom.h Ephemeris.h Sigma0.h Constants.h InstrumentSimAccurate.h AccurateGeom.h Antenna.h Wind.h GMF.h AntennaSim.h L00.h Spacecraft.h Distributions.h XTable.h Kpm.h InstConsts.h EarthPosition.h CoordinateSwitch.h LonLat.h Matrix3.h Meas.h BufferedList.h Beam.h Attitude.h Misc.h List.h Index.h PiscTable.h Kp.h BaseFile.h L00Frame.h EarthField.h Tracking.h Kpr.h
Interpolate.o: Interpolate.C 
Kp.o: Kp.C Kp.h Kpm.h Kpr.h Index.h EarthField.h Distributions.h Meas.h L00Frame.h LonLat.h Beam.h Ephemeris.h Attitude.h List.h EarthPosition.h CoordinateSwitch.h Tracking.h Matrix3.h BufferedList.h Constants.h
Kpm.o: Kpm.C Kpm.h Constants.h Array.h Index.h EarthField.h Distributions.h LonLat.h List.h EarthPosition.h Matrix3.h
Kpr.o: Kpr.C Kpr.h Constants.h Misc.h Array.h GenericGeom.h CoordinateSwitch.h Meas.h L00Frame.h EarthPosition.h Matrix3.h Attitude.h Beam.h LonLat.h Ephemeris.h List.h Tracking.h BufferedList.h
L00.o: L00.C L00.h BaseFile.h L00Frame.h Attitude.h
L00Frame.o: L00Frame.C L00Frame.h Attitude.h
L00ToL1A.o: L00ToL1A.C L00ToL1A.h L00.h L1A.h BaseFile.h L00Frame.h L1AFrame.h Attitude.h
L1A.o: L1A.C L1A.h BaseFile.h L1AFrame.h Attitude.h
L1AFrame.o: L1AFrame.C L1AFrame.h Attitude.h
L1AToL1B.o: L1AToL1B.C L1AToL1B.h InstrumentGeom.h Sigma0.h InstrumentSim.h L1A.h L1B.h Spacecraft.h Instrument.h Ephemeris.h XTable.h EarthPosition.h CoordinateSwitch.h LonLat.h Matrix3.h Meas.h Wind.h Antenna.h GMF.h AntennaSim.h L00.h Distributions.h Kpm.h BaseFile.h L1AFrame.h L1BFrame.h Constants.h Attitude.h InstConsts.h BufferedList.h List.h Beam.h Misc.h Index.h PiscTable.h Kp.h L00Frame.h EarthField.h Tracking.h Kpr.h
L1B.o: L1B.C L1B.h BaseFile.h L1BFrame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h Tracking.h EarthPosition.h Matrix3.h BufferedList.h Constants.h
L1BFrame.o: L1BFrame.C L1BFrame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h Tracking.h EarthPosition.h Matrix3.h BufferedList.h Constants.h
L1BToL2A.o: L1BToL2A.C L1BToL2A.h Antenna.h Ephemeris.h InstrumentGeom.h GenericGeom.h L1B.h Grid.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h BufferedList.h EarthPosition.h Spacecraft.h Instrument.h LonLat.h Meas.h BaseFile.h L1BFrame.h Array.h L2A.h Tracking.h List.h Constants.h InstConsts.h L2AFrame.h
L2A.o: L2A.C L2A.h BaseFile.h L2AFrame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h Tracking.h EarthPosition.h Matrix3.h BufferedList.h Constants.h
L2AFrame.o: L2AFrame.C L2AFrame.h Meas.h Misc.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h Tracking.h EarthPosition.h Matrix3.h BufferedList.h Constants.h
L2AToL2B.o: L2AToL2B.C L2AToL2B.h Constants.h Misc.h L2A.h L2B.h GMF.h BaseFile.h L2AFrame.h L2BFrame.h PiscTable.h Wind.h Meas.h Kp.h Beam.h List.h LonLat.h Index.h Ephemeris.h Attitude.h Kpm.h Kpr.h CoordinateSwitch.h Tracking.h EarthPosition.h Matrix3.h BufferedList.h EarthField.h Distributions.h L00Frame.h
L2B.o: L2B.C L2B.h BaseFile.h L2BFrame.h Wind.h Misc.h List.h LonLat.h Index.h EarthPosition.h Matrix3.h
L2BFrame.o: L2BFrame.C L2BFrame.h Wind.h Misc.h List.h LonLat.h Index.h EarthPosition.h Matrix3.h
List.o: List.C List.h
LonLat.o: LonLat.C LonLat.h Constants.h EarthPosition.h List.h Matrix3.h
Malloc1.o: Malloc1.C Malloc1.h
Matrix3.o: Matrix3.C Matrix3.h Constants.h
Meas.o: Meas.C Meas.h Beam.h Constants.h LonLat.h GenericGeom.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h Tracking.h EarthPosition.h Matrix3.h BufferedList.h
Misc.o: Misc.C Misc.h
PiscTable.o: PiscTable.C PiscTable.h Interpolate.h Constants.h Array.h Beam.h CoordinateSwitch.h Tracking.h Matrix3.h Attitude.h
Sigma0.o: Sigma0.C Sigma0.h Spacecraft.h Instrument.h Meas.h Distributions.h Constants.h Wind.h Matrix3.h Ephemeris.h Attitude.h Antenna.h InstConsts.h Beam.h LonLat.h List.h Misc.h Index.h BufferedList.h EarthPosition.h CoordinateSwitch.h Tracking.h
Spacecraft.o: Spacecraft.C Spacecraft.h Constants.h Matrix3.h Ephemeris.h Attitude.h BufferedList.h EarthPosition.h List.h
SpacecraftSim.o: SpacecraftSim.C Spacecraft.h SpacecraftSim.h Constants.h Matrix3.h Ephemeris.h Attitude.h Distributions.h BufferedList.h EarthPosition.h List.h
Tracking.o: Tracking.C Tracking.h Array.h Instrument.h Constants.h Antenna.h InstConsts.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h
Wind.o: Wind.C Wind.h Constants.h Misc.h Array.h List.h LonLat.h Index.h EarthPosition.h Matrix3.h
XTable.o: XTable.C XTable.h Constants.h Misc.h Array.h GenericGeom.h CoordinateSwitch.h EarthPosition.h Matrix3.h Attitude.h
