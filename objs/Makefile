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
	SimControl.o \
	Constants.o \
	Misc.o \
	List.o \
	ConfigList.o \
	GMF.o \
	OrbitSim.o \
	SimConfig.o \
	Instrument.o \
	AntennaSim.o

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

AntennaSim.o:	AntennaSim.C AntennaSim.h
ConfigList.o:	ConfigList.C ConfigList.h
Constants.o:	Constants.C Constants.h
GMF.o:			GMF.C GMF.h
Instrument.o:	Instrument.C Instrument.h
List.o:			List.C List.h
Misc.o:			Misc.C Misc.h
OrbitSim.o:		OrbitSim.C OrbitSim.h
SimConfig.o:	SimConfig.C SimConfig.h SimControl.h ConfigList.h
SimControl.o:	SimControl.C SimControl.h ConfigList.h
