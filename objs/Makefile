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
	LookGeom.o \
	Matrix3.o \
	Misc.o \
	Orbit.o \
	OrbitSim.o \
	Product.o

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

Antenna.o:		Antenna.C Antenna.h Beam.h
AntennaSim.o:	AntennaSim.C AntennaSim.h Constants.h
Beam.o:			Beam.C Beam.h
BeamSim.o:		BeamSim.C BeamSim.h
ConfigList.o:	ConfigList.C ConfigList.h List.h
ConfigSim.o:	ConfigSim.C ConfigSim.h InstrumentSim.h OrbitSim.h \
				ConfigList.h Instrument.h AntennaSim.h L0.h Orbit.h \
				List.h Antenna.h Constants.h Matrix3.h Beam.h Misc.h \
				Product.h
CoordinateSwitch.o:	CoordinateSwitch.C CoordinateSwitch.h Matrix3.h \
			Constants.h
GMF.o:			GMF.C GMF.h Interpolate.h
Instrument.o:	Instrument.C Instrument.h Antenna.h Orbit.h Beam.h \
				Constants.h Matrix3.h
InstrumentSim.o:	InstrumentSim.C InstrumentSim.h ConfigList.h \
				Instrument.h AntennaSim.h OrbitSim.h List.h Antenna.h \
				Orbit.h Beam.h Constants.h Matrix3.h
Interpolate.o:	Interpolate.C Interpolate.h
L0.o:			L0.C L0.h Product.h
L0ToL1.o:		L0ToL1.C L0ToL1.h
L1.o:			L1.C L1.h
List.o:			List.C List.h
LookGeom.o:		LookGeom.C LookGeom.h CoordinateSwitch.h \
			Matrix3.h Constants.h
Matrix3.o:		Matrix3.C Matrix3.h Constants.h
Misc.o:			Misc.C Misc.h
Orbit.o:		Orbit.C Orbit.h Constants.h Matrix3.h
OrbitSim.o:		OrbitSim.C OrbitSim.h Constants.h Orbit.h Matrix3.h
Product.o:		Product.C Product.h
