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
	GMF.o \
	Instrument.o \
	InstrumentSim.o \
	Misc.o \
	Orbit.o \
	OrbitSim.o

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
AntennaSim.o:	AntennaSim.C AntennaSim.h
Beam.o:			Beam.C Beam.h
BeamSim.o:		BeamSim.C BeamSim.h
ConfigList.o:	ConfigList.C ConfigList.h List.h
GMF.o:			GMF.C GMF.h
Instrument.o:	Instrument.C Instrument.h Antenna.h Orbit.h Beam.h \
				Constants.h
InstrumentSim.o:	InstrumentSim.C InstrumentSim.h ConfigList.h \
				Instrument.h OrbitSim.h List.h Antenna.h Orbit.h \
				Beam.h Constants.h
List.o:			List.C List.h
Misc.o:			Misc.C Misc.h
Orbit.o:		Orbit.C Orbit.h Constants.h
OrbitSim.o:		OrbitSim.C OrbitSim.h Constants.h Orbit.h
