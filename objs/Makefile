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
	GMF.o \
	Instrument.o \
	InstrumentSim.o \
	Interpolate.o \
	L0.o \
	L0ToL1.o \
	L1.o \
	L15.o \
	LookGeom.o \
	Matrix3.o \
	Measurement.o \
	Misc.o \
	Product.o \
	Spacecraft.o \
	SpacecraftSim.o \
	WindField.o \
	WindVector.o \
	WVC.o

#----------------------------------------------------------------------
# default: make library
#----------------------------------------------------------------------

default: $(TARGET_LIB)

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
AntennaSim.o: AntennaSim.C AntennaSim.h Constants.h Antenna.h Beam.h \
	Attitude.h
Attitude.o: Attitude.C Attitude.h
Beam.o: Beam.C Beam.h
BeamSim.o: BeamSim.C BeamSim.h
ConfigList.o: ConfigList.C ConfigList.h List.h
ConfigSim.o: ConfigSim.C ConfigSim.h InstrumentSim.h SpacecraftSim.h \
	Misc.h L0.h L1.h ConfigList.h Instrument.h AntennaSim.h Spacecraft.h \
	Product.h List.h Antenna.h Constants.h Matrix3.h Attitude.h Beam.h
CoordinateSwitch.o: CoordinateSwitch.C CoordinateSwitch.h Matrix3.h Attitude.h
GMF.o: GMF.C GMF.h Interpolate.h Constants.h Beam.h Measurement.h WVC.h \
	List.h WindVector.h
Instrument.o: Instrument.C Instrument.h Antenna.h Spacecraft.h Beam.h \
	Attitude.h Constants.h Matrix3.h
InstrumentSim.o: InstrumentSim.C InstrumentSim.h Instrument.h LookGeom.h \
	ConfigList.h AntennaSim.h SpacecraftSim.h L0.h Antenna.h Spacecraft.h \
	CoordinateSwitch.h List.h Product.h Beam.h Attitude.h Constants.h Matrix3.h
Interpolate.o: Interpolate.C 
L0.o: L0.C L0.h Product.h List.h
L0ToL1.o: L0ToL1.C L0ToL1.h L0.h L1.h Product.h List.h
L1.o: L1.C L1.h Product.h List.h
L15.o: L15.C L15.h Product.h List.h
L1ToL15.o: L1ToL15.C L1ToL15.h L1.h L15.h Product.h List.h
List.o: List.C List.h
LookGeom.o: LookGeom.C LookGeom.h CoordinateSwitch.h Constants.h Matrix3.h \
	Attitude.h
Matrix3.o: Matrix3.C Matrix3.h Constants.h
Measurement.o: Measurement.C Measurement.h Beam.h List.h
Misc.o: Misc.C Misc.h
Product.o: Product.C Product.h List.h
Spacecraft.o: Spacecraft.C Spacecraft.h Constants.h Matrix3.h Attitude.h
SpacecraftSim.o: SpacecraftSim.C SpacecraftSim.h Constants.h Spacecraft.h \
	Matrix3.h Attitude.h
WVC.o: WVC.C WVC.h List.h WindVector.h
WindField.o: WindField.C WindField.h WindVector.h
WindVector.o: WindVector.C WindVector.h
