#======================================================================
# @(#) $Id$
# Makefile for all SIM object code
#======================================================================

SHELL =			/bin/csh

INCDIR =		../../include
TARGET_LIB =	../../lib/libSim.a
CPPFLAGS =		-g -ptr. +w
SYSLIBS =		-lm

OBJS = \
	List.o \
	Antenna.o \
	AntennaSim.o \
	Attitude.o \
	Beam.o \
	BeamSim.o \
	ConfigList.o \
	ConfigSim.o \
	CoordinateSwitch.o \
	Ephemeris.o \
	GMF.o \
	GenericFile.o \
	Grid.o \
	Instrument.o \
	InstrumentSim.o \
	Interpolate.o \
	Kp.o \
	L00File.o \
	L00Frame.o \
	LonLat.o \
	LookGeom.o \
	Matrix3.o \
	Meas.o \
	Misc.o \
	PiscTable.o \
	Spacecraft.o \
	SpacecraftSim.o \
	WVC.o \
	WindField.o \
	WindVector.o

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
	- rm -f $(TARGET_LIB)
	- rm -f $(OBJS)
	- rm -rf Templates.DB

#----------------------------------------------------------------------
# install: same as default
#----------------------------------------------------------------------

install: $(TARGET_LIB)

#----------------------------------------------------------------------
# rules
#----------------------------------------------------------------------

$(TARGET_LIB):	$(OBJS)
	- rm -f $(TARGET_LIB)
	$(CCC) $(CPPFLAGS) -xar -o $(TARGET_LIB) $(OBJS)
	(set incs = (*.h); cd $(INCDIR); rm -f $$incs)
	cp *.h $(INCDIR)

%.C: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/objs/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/objs/RCS/$@,v

#----------------------------------------------------------------------
# Object file dependancies
#----------------------------------------------------------------------

Antenna.o: Antenna.C Antenna.h Beam.h Attitude.h
AntennaSim.o: AntennaSim.C AntennaSim.h Constants.h Antenna.h Beam.h Attitude.h
Attitude.o: Attitude.C Attitude.h
Beam.o: Beam.C Beam.h Attitude.h
BeamSim.o: BeamSim.C BeamSim.h
ConfigList.o: ConfigList.C ConfigList.h List.h
ConfigSim.o: ConfigSim.C ConfigSim.h InstrumentSim.h SpacecraftSim.h Misc.h L00File.h WindField.h ConfigList.h Instrument.h AntennaSim.h L00Frame.h GMF.h Ephemeris.h Spacecraft.h GenericFile.h WindVector.h List.h Antenna.h PiscTable.h Measurement.h WVC.h Matrix3.h Constants.h Beam.h Attitude.h
CoordinateSwitch.o: CoordinateSwitch.C CoordinateSwitch.h Matrix3.h Attitude.h
Ephemeris.o: Ephemeris.C Ephemeris.h Matrix3.h List.h
GMF.o: GMF.C GMF.h Interpolate.h Constants.h Beam.h PiscTable.h Measurement.h WVC.h Attitude.h List.h WindVector.h
GenericFile.o: GenericFile.C GenericFile.h
Grid.o: Grid.C Grid.h Meas.h LonLat.h Ephemeris.h Attitude.h List.h Matrix3.h
Instrument.o: Instrument.C Instrument.h Antenna.h Beam.h Attitude.h
InstrumentSim.o: InstrumentSim.C InstrumentSim.h Instrument.h LookGeom.h Ephemeris.h ConfigList.h AntennaSim.h L00Frame.h WindField.h GMF.h Antenna.h CoordinateSwitch.h Matrix3.h List.h WindVector.h PiscTable.h Measurement.h WVC.h Beam.h Attitude.h
Interpolate.o: Interpolate.C 
Kp.o: Kp.C Kp.h PiscTable.h Beam.h Attitude.h
L00File.o: L00File.C L00File.h GenericFile.h
L00Frame.o: L00Frame.C L00Frame.h
List.o: List.C List.h
LonLat.o: LonLat.C LonLat.h List.h
LookGeom.o: LookGeom.C LookGeom.h CoordinateSwitch.h Constants.h Matrix3.h Attitude.h
Matrix3.o: Matrix3.C Constants.h Matrix3.h
Meas.o: Meas.C Measurement.h Beam.h List.h Attitude.h
Misc.o: Misc.C Misc.h
PiscTable.o: PiscTable.C PiscTable.h Interpolate.h Constants.h Beam.h Attitude.h
Spacecraft.o: Spacecraft.C Spacecraft.h Constants.h Matrix3.h Ephemeris.h List.h
SpacecraftSim.o: SpacecraftSim.C Spacecraft.h SpacecraftSim.h Constants.h Matrix3.h Ephemeris.h List.h
WVC.o: WVC.C WVC.h Constants.h List.h WindVector.h
WindField.o: WindField.C WindField.h WindVector.h
WindVector.o: WindVector.C WindVector.h
