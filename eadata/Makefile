#
# @(#) $Id$
#

SHELL =       /bin/csh

.SUFFIXES: .C .o

CC =          g++
CDEBUGFLAGS = -O3
DEBUG =       NODEBUG

INCDIR =      ../../include
LIBDIR =      ../../lib
HDFINCDIR =   /usr/local/HDF/include

CFLAGS =      $(CDEBUGFLAGS) -D_NO_PROTO -Wall

C++FLAGS =    $(CDEBUGFLAGS) -D__SCAT_GNUCPP__ -DFUNCPROTO -D$(DEBUG) \
              -D_REENTRANT -DXTFUNCPROTO -D__EXTENSIONS__ \
              -I$(HDFINCDIR) -I. -fno-implicit-templates -DNOPM


OBJS = \
	Args.o \
	ArgsPlus.o \
	Backup.o \
	Binning.o \
	Command.o \
	CmdState.o \
	CommonDefs.o \
	Drn.o \
    DrvParamList.o \
	EALog.o \
	FrameRead.o \
	HdfFile.o \
	Headers.o \
	Hk2HdfMap.o \
    L1ADrvExtract.o \
    L1ADrvTab.o \
	L1AExtract.o \
	L1AFilTab.o \
	L1AFile.o \
	L1AFilter.o \
	L1AParTab.o \
	NoTimeTlmFile.o \
	L2AParTab.o \
	L2BParTab.o \
	LimitChecker.o \
	LimitList.o \
	LimitState.o \
	Itime.o \
	Mail.o \
	NoTimeTlmFile.o \
	Qpf.o \
	ParTab.o \
	Parameter.o \
	Polynomial.o \
	Print.o \
	TlmHdfFile.o \
	EAList.o \
	ListBase.o \
	ParameterList.o \
	CmdList.o \
	EAConfigList.o \
	Filter.o \
	PolyTable.o \
	Reqq.o \
	Rtcf.o \
    SafeString.o \
	TempInst.o \
	TlmFileList.o \
	ToDo.o \
	Eqx.o \
	Tpg.o


#	ErrorChecker.o \
#	HkdtParTab.o \
#	msgroute.o \
#	textfile.o \
#	PtrDoubleList.o \
#	DataSet.o \
#	GloblMsg.o
#	HkdtEffDetector.o \

.C.o:
	$(CC) -c $(C++FLAGS) $<

.c.o:
	cc -c $(CFLAGS) $<

default: $(LIBDIR)/libEAData.a

all:
	make clean
	make default

all_delivery:
	make clean
	make default

clean:
	-/bin/rm -f $(LIBDIR)/libEAData.a
	-/bin/rm -f $(OBJS)
	(set incs=(*.h EAList.C); cd $(INCDIR); /bin/rm -f $$incs)

$(LIBDIR)/libEAData.a: $(OBJS)
	-/bin/rm -f $@
	ar -r $@ $(OBJS); ranlib $@
	(set incs=(*.h EAList.C); cd $(INCDIR); /bin/rm -f $$incs)
	cp *.h $(INCDIR); cp EAList.C $(INCDIR)

%.C: $(IO_CENTRAL_TREE)/src/data/RCS/%.C,v
	co $(IO_CENTRAL_TREE)/src/data/RCS/$@,v
 
%.h: $(IO_CENTRAL_TREE)/src/data/RCS/%.h,v
	co $(IO_CENTRAL_TREE)/src/data/RCS/$@,v

Args.o:			Args.C Args.h EAConfigList.h EAList.h

ArgsPlus.o:		ArgsPlus.C ArgsPlus.h ArgDefs.h Args.h ParTab.h CmdList.h \
				Filter.h Parameter.h Eqx.h LimitList.h TlmFileList.h \
				Command.h Itime.h Tpg.h LimitChecker.h TlmFile.h \
				ParameterList.h LimitState.h ToDo.h CommonDefs.h EAList.C
Backup.o:		Backup.C Backup.h
Binning.o:		Binning.C Binning.h NSCAT.h
EffDetector.o:  EffDetector.C EffDetector.h CmdList.h EAList.h Command.h \
	            CommonDefs.h Itime.h Tpg.h L1AExtract.h HkdtExtract.h \
                State.h Eqx.h Parameter.h 
Command.o:      Command.C Command.h CommonDefs.h Itime.h Tpg.h
CmdList.o:      CmdList.C CmdList.h Command.h CmdState.h EAList.h EAList.C Tpg.h \
	            Eqx.h Itime.h Parameter.h
CmdState.o:     CmdState.C CmdState.h Command.h Parameter.h CommonDefs.h \
				Itime.h Tpg.h EAList.h
CommonDefs.o:   CommonDefs.C CommonDefs.h Parameter.h
EAConfigList.o: EAConfigList.C EAConfigList.h EAList.h EAList.C CommonDefs.h
DataSet.o:      DataSet.C DataSet.h Itime.h ParTab.h
Drn.o:			Drn.C Drn.h EAConfigList.h Itime.h Mail.h EAList.h \
				CommonDefs.h
Headers.o:		Headers.C Headers.h Itime.h Parameter.h
L1AEffDetector.o:  L1AEffDetector.C EffDetector.h L1AEffDetector.h \
	                 Command.h
L1AErrorChecker.o: L1AErrorChecker.C L1AErrorChecker.h L1AExtract.h \
	                 Parameter.h State.h ErrorChecker.h
L1AExtract.o: L1AExtract.C L1AExtract.h Parameter.h Itime.h
L1AFilTab.o:  L1AFilTab.C L1AFilter.h Filter.h
L1AFile.o:    L1AFile.C L1AFile.h L1AExtract.h TlmFile.h
L1AFilter.o:  L1AFilter.C L1AExtract.h Parameter.h
L1AParTab.o:  	L1AParTab.C ParTab.h L1AExtract.h Print.h ParameterList.h \
				Parameter.h CommonDefs.h Itime.h
Eqx.o:          Eqx.C Eqx.h Itime.h EAList.h
ErrorChecker.o: ErrorChecker.C ErrorChecker.h Parameter.h State.h
Filter.o:       Filter.C Filter.h EAList.h EAList.C
GloblMsg.o:     GloblMsg.C GloblMsg.h 
HdfFile.o:		HdfFile.h EAList.h HdfFile.C
HkdtEffDetector.o:  HkdtEffDetector.C EffDetector.h HkdtEffDetector.h \
	                Command.h
HkdtErrorChecker.o: HkdtErrorChecker.C HkdtErrorChecker.h HkdtExtract.h \
	                Parameter.h State.h
HkdtExtract.o:  HkdtExtract.C HkdtExtract.h Itime.h Parameter.h
HkdtFilTab.o:   HkdtFilTab.C HkdtFilter.h Filter.h
HkdtFile.o:     HkdtFile.C HkdtFile.h HkdtExtract.h Itime.h
HkdtFilter.o:   HkdtFilter.C HkdtExtract.h Parameter.h
HkdtParTab.o:   HkdtParTab.C ParTab.h HkdtExtract.h Print.h ParameterList.h \
					Parameter.h CommonDefs.h Itime.h
Itime.o:        Itime.C Itime.h CommonDefs.h
LimitChecker.o: LimitChecker.C LimitChecker.h LimitState.h \
	            HkdtExtract.h L1AExtract.h Parameter.h Itime.h
LimitList.o:    LimitList.C CommonDefs.h LimitList.h ParTab.h \
	            LimitChecker.h LimitState.h HkdtExtract.h L1AExtract.h \
	            Parameter.h EAList.h EAList.C Itime.h
LimitState.o:   LimitState.C LimitState.h HkdtExtract.h L1AExtract.h
EAList.o:       EAList.C EAList.h
Mail.o:			Mail.C Mail.h
Npf.o:          Npf.C Npf.h Itime.h Command.h CmdList.h Binning.h \
				CommonDefs.h Tpg.h EAList.h Eqx.h
ParTab.o:       ParTab.C Parameter.h ParTab.h Itime.h
Parameter.o:    Parameter.C Parameter.h
ParameterList.o:    ParameterList.C ParameterList.h Itime.h ParTab.h \
					Parameter.h EAList.h CommonDefs.h
Polynomial.o:	CommonDefs.h Polynomial.h
PolyTable.o:	EAList.h Polynomial.h CommonDefs.h PolyTable.h
Print.o:        Print.C Print.h Itime.h
PtrDoubleList.o: PtrDoubleList.C PtrDoubleList.h
Reqq.o:         Reqq.C Reqq.h Command.h CmdList.h CommonDefs.h Itime.h \
				Tpg.h EAList.h Eqx.h
TlmHdfFile.o:	HdfFile.h Parameter.h Itime.h TlmHdfFile.C
ToDo.o:			ToDo.h ToDo.C Itime.h EAList.h EAList.C
Tpg.o:          Tpg.C Tpg.h EAList.h Itime.h
Rtcf.o:         Rtcf.C Rtcf.h Command.h CmdList.h CommonDefs.h Itime.h \
				Tpg.h EAList.h Eqx.h
State.o:        State.C State.h
TlmFile.o:      TlmFile.C TlmFile.h
TlmFileList.o:  TlmFileList.C TlmFileList.h TlmHdfFile.h \
	            EAList.h EAList.C TlmHdfFile.h Itime.h Parameter.h
msgroute.o:     msgroute.C msgroute.h
textfile.o:     textfile.C textfile.h
TempInst.o:		EAList.h EAList.C TempInst.C

# dependency rules for header files
#LimitChecker.h: Parameter.h Itime.h LimitState.h
