#======================================================================
# @(#) $Id$
# Makefile for all SIM object code
#======================================================================

SHELL =			/bin/csh

INCDIR =		../../include
TARGET_LIB =	../../lib/libSim.a
CPPFLAGS =		-g -fno-implicit-templates -Wall
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
	EarthPosition.o \
	Ephemeris.o \
	GMF.o \
	GenericFile.o \
	GenericGeom.o \
	Grid.o \
	Instrument.o \
	InstrumentSim.o \
	InstrumentSimAccurate.o \
	InstrumentGeom.o \
	Interpolate.o \
	KprTable.o \
	L00.o \
	L00Frame.o \
	L00ToL10.o \
	L10.o \
	L10Frame.o \
	L10ToL15.o \
	L15.o \
	L15Frame.o \
	L15ToL17.o \
	L17.o \
	L17Frame.o \
	L17ToL20.o \
	L20.o \
	L20Frame.o \
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
	Wind.o

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

AccurateGeom.o: AccurateGeom.C CoordinateSwitch.h Ephemeris.h Attitude.h Antenna.h Matrix3.h GenericGeom.h Spacecraft.h Instrument.h LonLat.h InstrumentGeom.h AccurateGeom.h Interpolate.h Array.h Misc.h BufferedList.h EarthPosition.h Beam.h Constants.h List.h Meas.h
Antenna.o: Antenna.C Antenna.h Beam.h Constants.h CoordinateSwitch.h Attitude.h Matrix3.h
AntennaSim.o: AntennaSim.C AntennaSim.h Constants.h Antenna.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h
Array.o: Array.C Array.h
Attitude.o: Attitude.C Attitude.h
BaseFile.o: BaseFile.C BaseFile.h
Beam.o: Beam.C Beam.h Array.h Constants.h Attitude.h CoordinateSwitch.h Matrix3.h
BeamSim.o: BeamSim.C BeamSim.h
BufferedList.o: BufferedList.C BufferedList.h List.h
ConfigList.o: ConfigList.C ConfigList.h List.h
ConfigSim.o: ConfigSim.C ConfigSim.h InstrumentSim.h InstrumentSimAccurate.h SpacecraftSim.h Misc.h L00.h L10.h L15.h L17.h L20.h Constants.h Distributions.h Tracking.h L17ToL20.h Wind.h Grid.h ConfigList.h Antenna.h Instrument.h GMF.h AntennaSim.h Spacecraft.h GenericFile.h L00Frame.h L10Frame.h BaseFile.h L15Frame.h L17Frame.h L20Frame.h List.h LonLat.h Array.h Meas.h CoordinateSwitch.h Beam.h Attitude.h PiscTable.h Matrix3.h Ephemeris.h EarthPosition.h BufferedList.h
CoordinateSwitch.o: CoordinateSwitch.C CoordinateSwitch.h Matrix3.h Attitude.h
Distributions.o: Distributions.C Distributions.h
EarthPosition.o: EarthPosition.C EarthPosition.h Constants.h LonLat.h CoordinateSwitch.h Matrix3.h List.h Attitude.h
Ephemeris.o: Ephemeris.C Ephemeris.h List.h List.C Interpolate.h Matrix3.h BufferedList.h EarthPosition.h
GMF.o: GMF.C GMF.h Interpolate.h Constants.h Beam.h PiscTable.h Wind.h Meas.h CoordinateSwitch.h Misc.h List.h LonLat.h Ephemeris.h Attitude.h Matrix3.h EarthPosition.h BufferedList.h
GenericFile.o: GenericFile.C GenericFile.h
GenericGeom.o: GenericGeom.C GenericGeom.h CoordinateSwitch.h Constants.h EarthPosition.h Matrix3.h Attitude.h
Grid.o: Grid.C Grid.h Meas.h Array.h L15.h L17.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h BaseFile.h L15Frame.h GenericFile.h L17Frame.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
Instrument.o: Instrument.C Instrument.h Antenna.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h
InstrumentGeom.o: InstrumentGeom.C CoordinateSwitch.h Ephemeris.h Attitude.h Antenna.h Matrix3.h GenericGeom.h Spacecraft.h Instrument.h LonLat.h InstrumentGeom.h Interpolate.h Array.h Misc.h Constants.h BufferedList.h EarthPosition.h Beam.h List.h Meas.h
InstrumentSim.o: InstrumentSim.C InstrumentSim.h Instrument.h GenericGeom.h InstrumentGeom.h Ephemeris.h Sigma0.h Constants.h Antenna.h Wind.h GMF.h AntennaSim.h L00.h Spacecraft.h Distributions.h EarthPosition.h CoordinateSwitch.h LonLat.h Matrix3.h Meas.h BufferedList.h Beam.h Attitude.h Misc.h List.h PiscTable.h GenericFile.h L00Frame.h
InstrumentSimAccurate.o: InstrumentSimAccurate.C InstrumentSim.h Instrument.h GenericGeom.h InstrumentGeom.h Ephemeris.h Sigma0.h Constants.h InstrumentSimAccurate.h AccurateGeom.h Antenna.h Wind.h GMF.h AntennaSim.h L00.h Spacecraft.h Distributions.h EarthPosition.h CoordinateSwitch.h LonLat.h Matrix3.h Meas.h BufferedList.h Beam.h Attitude.h Misc.h List.h PiscTable.h GenericFile.h L00Frame.h
Interpolate.o: Interpolate.C 
Kp.o: Kp.C Kp.h PiscTable.h Beam.h CoordinateSwitch.h Matrix3.h Attitude.h
KprTable.o: KprTable.C KprTable.h Constants.h Misc.h Array.h GenericGeom.h CoordinateSwitch.h Meas.h L00Frame.h EarthPosition.h Matrix3.h Attitude.h Beam.h LonLat.h Ephemeris.h List.h BufferedList.h
L00.o: L00.C L00.h GenericFile.h L00Frame.h Attitude.h
L00Frame.o: L00Frame.C L00Frame.h Attitude.h
L00ToL10.o: L00ToL10.C L00ToL10.h L00.h L10.h GenericFile.h L00Frame.h L10Frame.h Attitude.h
L10.o: L10.C L10.h GenericFile.h L10Frame.h Attitude.h
L10Frame.o: L10Frame.C L10Frame.h Attitude.h
L10ToL15.o: L10ToL15.C L10ToL15.h InstrumentGeom.h Sigma0.h L10.h L15.h Spacecraft.h Instrument.h Ephemeris.h EarthPosition.h CoordinateSwitch.h LonLat.h Matrix3.h Meas.h GenericFile.h L10Frame.h BaseFile.h L15Frame.h Constants.h Attitude.h Antenna.h BufferedList.h List.h Beam.h
L15.o: L15.C L15.h BaseFile.h L15Frame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L15Frame.o: L15Frame.C L15Frame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L15ToL17.o: L15ToL17.C L15ToL17.h Antenna.h Ephemeris.h InstrumentGeom.h GenericGeom.h L15.h Grid.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h BufferedList.h EarthPosition.h Spacecraft.h Instrument.h LonLat.h Meas.h BaseFile.h L15Frame.h Array.h L17.h List.h Constants.h GenericFile.h L17Frame.h
L17.o: L17.C L17.h GenericFile.h L17Frame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L17Frame.o: L17Frame.C L17Frame.h Meas.h Beam.h LonLat.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L17ToL20.o: L17ToL20.C L17ToL20.h Constants.h Misc.h L17.h L20.h GMF.h GenericFile.h L17Frame.h BaseFile.h L20Frame.h PiscTable.h Wind.h Meas.h Beam.h List.h LonLat.h Ephemeris.h Attitude.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
L20.o: L20.C L20.h BaseFile.h L20Frame.h Wind.h Misc.h List.h LonLat.h EarthPosition.h Matrix3.h
L20Frame.o: L20Frame.C L20Frame.h Wind.h Misc.h List.h LonLat.h EarthPosition.h Matrix3.h
List.o: List.C List.h
LonLat.o: LonLat.C LonLat.h Constants.h EarthPosition.h List.h Matrix3.h
Malloc1.o: Malloc1.C Malloc1.h
Matrix3.o: Matrix3.C Matrix3.h Constants.h
Meas.o: Meas.C Meas.h Beam.h Constants.h LonLat.h GenericGeom.h Ephemeris.h Attitude.h List.h CoordinateSwitch.h EarthPosition.h Matrix3.h BufferedList.h
Misc.o: Misc.C Misc.h
PiscTable.o: PiscTable.C PiscTable.h Interpolate.h Constants.h Array.h Beam.h CoordinateSwitch.h Matrix3.h Attitude.h
Sigma0.o: Sigma0.C Sigma0.h Spacecraft.h Instrument.h Meas.h Distributions.h Constants.h Matrix3.h Ephemeris.h Attitude.h Antenna.h Beam.h LonLat.h List.h BufferedList.h EarthPosition.h CoordinateSwitch.h
Spacecraft.o: Spacecraft.C Spacecraft.h Constants.h Matrix3.h Ephemeris.h Attitude.h BufferedList.h EarthPosition.h List.h
SpacecraftSim.o: SpacecraftSim.C Spacecraft.h SpacecraftSim.h Constants.h Matrix3.h Ephemeris.h Attitude.h Distributions.h BufferedList.h EarthPosition.h List.h
Tracking.o: Tracking.C Tracking.h Array.h Constants.h Instrument.h Antenna.h CoordinateSwitch.h Beam.h Attitude.h Matrix3.h
Wind.o: Wind.C Wind.h Constants.h Misc.h Array.h List.h LonLat.h EarthPosition.h Matrix3.h
