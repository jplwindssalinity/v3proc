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
HDFINCDIR =   $(INCDIR)/hdf

CFLAGS =      $(CDEBUGFLAGS) -D_NO_PROTO -Wall

C++FLAGS =    $(CDEBUGFLAGS) -D__SCAT_GNUCPP__ -DFUNCPROTO -D$(DEBUG) \
              -D_REENTRANT -DXTFUNCPROTO -D__EXTENSIONS__ \
              -I$(HDFINCDIR) -I. -fno-implicit-templates -DNOPM


OBJS = \
	TempInst.o \
	Args.o \
	ArgsPlus.o \
	Backup.o \
	Binning.o \
	Command.o \
	CmdList.o \
	CmdEffect.o \
	CommonDefs.o \
	EAConfigList.o \
	EffDetector.o \
	ErrorChecker.o \
	FrameRead.o \
	Drn.o \
	DrvParamList.o \
	EAList.o \
	EALog.o \
	Eqx.o \
	FilterFunc.o \
	Filter.o \
	HdfFile.o \
	Headers.o \
	Hk2ErrorChecker.o \
	Hk2Extract.o \
	Hk2File.o \
	Hk2FilTab.o \
	Hk2HdfMap.o \
	Hk2ParTab.o \
	Itime.o \
	L1ADrvExtract.o \
	L1ADrvTab.o \
	L1AEffDetector.o \
	L1AErrorChecker.o \
	L1AExtract.o \
	L1AFile.o \
	NoTimeTlmFile.o \
	L1AFilTab.o \
	L1AParTab.o \
	L1BHdfFile.o \
	L1BParTab.o \
	L2AParTab.o \
	L2BParTab.o \
	LimitChecker.o \
	LimitList.o \
	LimitState.o \
	ListBase.o \
	Mail.o \
	Qpa.o \
	Qpf.o \
	ParTab.o \
	Parameter.o \
	ParameterList.o \
	Polynomial.o \
	PolyTable.o \
	Print.o \
	Reqa.o \
	Reqq.o \
	ReqaqList.o \
	Rtcf.o \
	SafeString.o \
	State.o \
	TimeTlmFile.o \
	TlmHdfFile.o \
	TlmFileList.o \
	ToDo.o \
	Tpg.o

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

%.C: $(SIM_CENTRAL_TREE)/src/eadata/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/eadata/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/eadata/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/eadata/RCS/$@,v

.PRECIOUS: %.C

Args.o: Args.C Args.h ArgDefs.h EAConfigList.h EALog.h EAList.h TlmFileList.h TlmHdfFile.h ListBase.h Parameter.h Itime.h CommonDefs.h HdfFile.h
ArgsPlus.o: ArgsPlus.C Args.h ArgsPlus.h ParTab.h EAConfigList.h EALog.h TlmFileList.h LimitList.h Filter.h ToDo.h CmdList.h Eqx.h PolyTable.h ArgDefs.h Itime.h Parameter.h CommonDefs.h EAList.h TlmHdfFile.h LimitChecker.h Command.h Tpg.h Polynomial.h PolyErrNo.h ListBase.h HdfFile.h LimitState.h
Backup.o: Backup.C Backup.h
Binning.o: Binning.C Binning.h NSCAT.h
CmdList.o: CmdList.C Command.h CmdList.h CmdState.h Eqx.h Itime.h CommonDefs.h Tpg.h EAList.h Parameter.h ListBase.h
CmdState.o: CmdState.C CmdState.h Command.h Parameter.h CommonDefs.h Tpg.h Itime.h
Command.o: Command.C Command.h Tpg.h CommonDefs.h Itime.h MacroCommand.h
CommonDefs.o: CommonDefs.C Parameter.h CommonDefs.h
Drn.o: Drn.C Drn.h Itime.h Mail.h EAConfigList.h CommonDefs.h EAList.h ListBase.h
DrvParamList.o: DrvParamList.C DrvParamList.h Itime.h ParTab.h PolyTable.h TlmHdfFile.h ParameterList.h CommonDefs.h Parameter.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h
EAConfigList.o: EAConfigList.C CommonDefs.h EAConfigList.h EAList.h ListBase.h
EAList.o: EAList.C EAList.h ListBase.h
EALog.o: EALog.C EALog.h TlmFileList.h TlmHdfFile.h EAList.h Parameter.h Itime.h CommonDefs.h HdfFile.h ListBase.h
EffDetector.o: EffDetector.h L1AExtract.h HkdtExtract.h
Eqx.o: Eqx.C Eqx.h Itime.h EAList.h Tpg.h CommonDefs.h ListBase.h
Filter.o: Filter.C Filter.h ParTab.h EAList.h TlmHdfFile.h CommonDefs.h Parameter.h ListBase.h HdfFile.h Itime.h
FrameRead.o: FrameRead.C CommonDefs.h
HdfFile.o: HdfFile.C HdfFile.h EAList.h ListBase.h
Headers.o: Headers.C Headers.h Parameter.h Itime.h CommonDefs.h
Hk2ErrorChecker.o: Hk2ErrorChecker.C Hk2ErrorChecker.h
Hk2File.o: Hk2File.C Hk2File.h TimeTlmFile.h
Hk2FilTab.o: Hk2FilTab.C FilterFunc.h
Hk2HdfMap.o: Hk2HdfMap.C Parameter.h Hk2HdfMap.h FrameRead.h CommonDefs.h
Hk2ParTab.o: Hk2ParTab.C Hk2Extract.h Print.h
Itime.o: Itime.C Itime.h CommonDefs.h
L1ADrvExtract.o: L1ADrvExtract.C L1AExtract.h L1ADrvExtract.h Parameter.h Itime.h ArgsPlus.h PolyTable.h Polynomial.h CommonDefs.h TlmHdfFile.h Args.h ArgDefs.h EALog.h EAList.h PolyErrNo.h HdfFile.h EAConfigList.h TlmFileList.h ListBase.h
L1ADrvTab.o: L1ADrvTab.C ParTab.h L1AExtract.h L1ADrvExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
L1AEffDetector.o: L1AEffDetector.C L1AEffDetector.h
L1AExtract.o: L1AExtract.C L1AExtract.h Parameter.h Itime.h CommonDefs.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
L1AFile.o: L1AFile.C L1AFile.h L1AExtract.h ParTab.h TlmHdfFile.h Itime.h Parameter.h CommonDefs.h HdfFile.h EAList.h ListBase.h
L1AFilTab.o: L1AFilTab.C Filter.h L1AFilter.h EAList.h TlmHdfFile.h ListBase.h CommonDefs.h HdfFile.h Itime.h Parameter.h
L1AFilter.o: L1AFilter.C Filter.h L1AExtract.h Parameter.h ParTab.h EAList.h TlmHdfFile.h CommonDefs.h Itime.h ListBase.h HdfFile.h
L1AParTab.o: L1AParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
L1BHdfFile.o: L1BHdfFile.C L1BHdfFile.h
L2AParTab.o: L2AParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
L2BParTab.o: L2BParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
LimitChecker.o: LimitChecker.C LimitChecker.h LimitState.h HkdtExtract.h L1AExtract.h ParTab.h Parameter.h Itime.h CommonDefs.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
LimitList.o: LimitList.C CommonDefs.h LimitList.h ParTab.h LimitChecker.h LimitState.h HkdtExtract.h L1AExtract.h Parameter.h EAList.h Itime.h TlmHdfFile.h ListBase.h HdfFile.h
LimitState.o: LimitState.C LimitState.h HkdtExtract.h L1AExtract.h ParTab.h TlmHdfFile.h Parameter.h CommonDefs.h Itime.h HdfFile.h EAList.h ListBase.h
ListBase.o: ListBase.C ListBase.h
Mail.o: Mail.C Mail.h EAConfigList.h EAList.h ListBase.h
NoTimeTlmFile.o: NoTimeTlmFile.C NoTimeTlmFile.h Parameter.h TlmHdfFile.h CommonDefs.h HdfFile.h Itime.h EAList.h ListBase.h
Parameter.o: Parameter.C Parameter.h CommonDefs.h
ParameterList.o: ParameterList.C ParameterList.h Itime.h ParTab.h PolyTable.h TlmHdfFile.h CommonDefs.h Parameter.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h
ParTab.o: ParTab.C Parameter.h ParTab.h Itime.h HdfFile.h CommonDefs.h EAList.h ListBase.h
Polynomial.o: Polynomial.C Polynomial.h CommonDefs.h
PolyTable.o: PolyTable.C CommonDefs.h PolyTable.h EAList.h Polynomial.h PolyErrNo.h ListBase.h
Print.o: Print.C Print.h
Qpa.o: Qpa.C Qpf.h Qpa.h
Qpf.o: Qpf.C Qpf.h Itime.h Command.h CmdList.h Binning.h CommonDefs.h Tpg.h EAList.h Eqx.h Parameter.h ListBase.h
Reqa.o: Reqa.C Reqa.h
Reqq.o: Reqq.C CommonDefs.h Reqq.h Command.h CmdList.h Binning.h Tpg.h EAList.h Eqx.h Itime.h Parameter.h ListBase.h
Rtcf.o: Rtcf.C CommonDefs.h Command.h Rtcf.h Reqq.h Binning.h Tpg.h CmdList.h Itime.h EAList.h Eqx.h Parameter.h ListBase.h
SafeString.o: SafeString.C 
TempInst.o: TempInst.C EAList.h EAList.C Command.h EAConfigList.h Eqx.h Filter.h LimitChecker.h Parameter.h Polynomial.h TlmHdfFile.h ToDo.h ListBase.h CommonDefs.h Tpg.h Itime.h LimitState.h HdfFile.h SafeString.h L1AErrorChecker.h LimitChecker.h ErrorChecker.h ReqaqList.h State.h Reqq.h CmdList.h CmdEffect.h
TlmFileList.o: TlmFileList.C TlmFileList.h L1AFile.h NoTimeTlmFile.h EAList.h TlmHdfFile.h Parameter.h Itime.h ListBase.h CommonDefs.h HdfFile.h
TlmHdfFile.o: TlmHdfFile.C TlmHdfFile.h CommonDefs.h HdfFile.h Itime.h Parameter.h EAList.h ListBase.h
ToDo.o: ToDo.C ToDo.h CommonDefs.h Itime.h EAList.h ListBase.h
Tpg.o: Tpg.C Tpg.h Itime.h CommonDefs.h
