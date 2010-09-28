#
# @(#) $Id$
#

SHELL =       /bin/csh

.SUFFIXES: .C .o

CC =          g++
CDEBUGFLAGS = -O2
DEBUG =       NODEBUG

INCDIR =      ../../include
MALLOC_INCDIR =         /usr/include/malloc
LIBDIR =      ../../lib
ifneq '$(shell uname -s)' 'Linux'
    HDFINCDIR =   /sw/include
else
    HDFINCDIR = /usr/include/hdf
endif

INCFLAGS =    -I$(HDFINCDIR) -I$(MALLOC_INCDIR) -I.

CFLAGS =      $(CDEBUGFLAGS) -D_NO_PROTO -Wall -Wno-deprecated

C++FLAGS =    $(CDEBUGFLAGS) -D__SCAT_GNUCPP__ -DFUNCPROTO -D$(DEBUG) \
              -D_REENTRANT -DXTFUNCPROTO -D__EXTENSIONS__ \
              $(INCFLAGS) -fno-implicit-templates -DNOPM -Wno-deprecated

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
	Hk2EffDetector.o \
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
	L2AhrParTab.o \
	L2AxParTab.o \
	L2BParTab.o \
	LeapSecTable.o \
	LimitChecker.o \
	LimitList.o \
	LimitState.o \
	ListBase.o \
	Mail.o \
	MemoryFromFile.o \
	MemoryFromTlm.o \
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
	Tpg.o \
	UpldTbl.o \
	UpldTblList.o

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

#----------------------------------------------------------------------
# depend: list dependencies
#----------------------------------------------------------------------

depend:
	@ inclist `echo $(OBJS) | sed -e 's/[.]o/.C/g'`

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

TempInst.o: TempInst.C EAList.h EAList.C Command.h EAConfigList.h Eqx.h Filter.h LimitChecker.h L1AErrorChecker.h Parameter.h Polynomial.h ReqaqList.h TlmHdfFile.h ToDo.h ListBase.h CommonDefs.h Tpg.h Itime.h LimitState.h ErrorChecker.h Reqq.h HdfFile.h State.h CmdList.h SafeString.h CmdEffect.h LeapSecTable.h
Args.o: Args.C Args.h ArgDefs.h EAConfigList.h EALog.h EAList.h TlmFileList.h TlmHdfFile.h ListBase.h Parameter.h Itime.h CommonDefs.h HdfFile.h SafeString.h
ArgsPlus.o: ArgsPlus.C Args.h ArgsPlus.h ParTab.h EAConfigList.h EALog.h TlmFileList.h LimitList.h Filter.h ToDo.h CmdList.h Eqx.h PolyTable.h ArgDefs.h Itime.h Parameter.h CommonDefs.h EAList.h TlmHdfFile.h LimitChecker.h Command.h CmdEffect.h Tpg.h Polynomial.h PolyErrNo.h ListBase.h HdfFile.h LimitState.h SafeString.h
Backup.o: Backup.C Backup.h
Binning.o: Binning.C Binning.h NSCAT.h
Command.h: Command_1.h UpldTbl.h MemoryFromFile.h MemoryFromTlm.h
Command.o: Command.C Command.h MacroCommand.h Tpg.h CommonDefs.h Itime.h
CmdList.o: CmdList.C Reqq.h CmdList.h Command.h EAList.h CmdEffect.h Eqx.h Itime.h Parameter.h CommonDefs.h Tpg.h ListBase.h
CmdEffect.o: CmdEffect.C CmdEffect.h Command.h Parameter.h CommonDefs.h Tpg.h Itime.h
CommonDefs.o: CommonDefs.C Parameter.h CommonDefs.h
EAConfigList.o: EAConfigList.C CommonDefs.h EAConfigList.h EAList.h ListBase.h
EffDetector.o: EffDetector.C EffDetector.h L1AExtract.h HkdtExtract.h CmdList.h TlmFileList.h Parameter.h State.h CommonDefs.h Itime.h TlmHdfFile.h EAList.h Command.h CmdEffect.h Eqx.h HdfFile.h ListBase.h Tpg.h SafeString.h
ErrorChecker.o: ErrorChecker.C Parameter.h ErrorChecker.h State.h CommonDefs.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
FrameRead.o: FrameRead.C CommonDefs.h
Drn.o: Drn.C Drn.h Itime.h Mail.h EAConfigList.h CommonDefs.h EAList.h ListBase.h
DrvParamList.o: DrvParamList.C DrvParamList.h Itime.h ParTab.h PolyTable.h TlmHdfFile.h ParameterList.h CommonDefs.h Parameter.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h SafeString.h
EAList.o: EAList.C EAList.h ListBase.h
EALog.o: EALog.C EALog.h TlmFileList.h TlmHdfFile.h EAList.h Parameter.h Itime.h CommonDefs.h HdfFile.h ListBase.h SafeString.h
Eqx.o: Eqx.C Eqx.h Itime.h EAList.h Tpg.h CommonDefs.h ListBase.h
FilterFunc.o: FilterFunc.C Filter.h L1AExtract.h Parameter.h ParTab.h EAList.h TlmHdfFile.h CommonDefs.h Itime.h ListBase.h HdfFile.h SafeString.h
Filter.o: Filter.C Filter.h ParTab.h EAList.h TlmHdfFile.h CommonDefs.h Parameter.h ListBase.h HdfFile.h Itime.h SafeString.h
HdfFile.o: HdfFile.C HdfFile.h Parameter.h EAList.h SafeString.h CommonDefs.h ListBase.h
Headers.o: Headers.C Headers.h Parameter.h Itime.h CommonDefs.h
Hk2EffDetector.o: Hk2EffDetector.C Hk2EffDetector.h Itime.h EALog.h Parameter.h ParTab.h State.h TlmHdfFile.h ErrorChecker.h
Hk2ErrorChecker.o: Hk2ErrorChecker.C Hk2ErrorChecker.h L1AExtract.h Parameter.h ParTab.h State.h Itime.h ErrorChecker.h CommonDefs.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
Hk2Extract.o: Hk2Extract.C CommonDefs.h L1AExtract.h Parameter.h PolyTable.h Itime.h TlmHdfFile.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h SafeString.h
Hk2File.o: Hk2File.C Hk2File.h L1AExtract.h ParTab.h TimeTlmFile.h CommonDefs.h Itime.h TlmHdfFile.h Parameter.h HdfFile.h EAList.h SafeString.h ListBase.h
Hk2FilTab.o: Hk2FilTab.C Filter.h FilterFunc.h EAList.h TlmHdfFile.h ListBase.h CommonDefs.h HdfFile.h Itime.h Parameter.h SafeString.h
Hk2HdfMap.o: Hk2HdfMap.C Parameter.h Hk2HdfMap.h FrameRead.h CommonDefs.h
Hk2ParTab.o: Hk2ParTab.C ParTab.h L1AExtract.h Hk2Extract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
Itime.o: Itime.C Itime.h CommonDefs.h LeapSecTable.h
L1ADrvExtract.o: L1ADrvExtract.C L1AExtract.h L1ADrvExtract.h Parameter.h Itime.h PolyTable.h Polynomial.h CommonDefs.h TlmHdfFile.h EAList.h PolyErrNo.h HdfFile.h ListBase.h SafeString.h
L1ADrvTab.o: L1ADrvTab.C ParTab.h L1AExtract.h L1ADrvExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h PolyTable.h HdfFile.h EAList.h Polynomial.h PolyErrNo.h SafeString.h ListBase.h
L1AEffDetector.o: L1AEffDetector.C L1AEffDetector.h Itime.h EffDetector.h EALog.h Parameter.h ParTab.h State.h TlmHdfFile.h CommonDefs.h CmdList.h TlmFileList.h HdfFile.h EAList.h Command.h CmdEffect.h Eqx.h SafeString.h ListBase.h Tpg.h UpldTblList.h
L1AErrorChecker.o: L1AErrorChecker.C L1AErrorChecker.h L1AExtract.h Parameter.h ParTab.h State.h Itime.h EAList.h ErrorChecker.h CommonDefs.h TlmHdfFile.h ListBase.h HdfFile.h SafeString.h
L1AExtract.o: L1AExtract.C L1AExtract.h Parameter.h PolyTable.h Itime.h CommonDefs.h TlmHdfFile.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h SafeString.h
L1AFile.o: L1AFile.C L1AFile.h L1AExtract.h ParTab.h TimeTlmFile.h CommonDefs.h Itime.h TlmHdfFile.h Parameter.h HdfFile.h EAList.h SafeString.h ListBase.h
NoTimeTlmFile.o: NoTimeTlmFile.C NoTimeTlmFile.h Parameter.h TlmHdfFile.h CommonDefs.h HdfFile.h Itime.h EAList.h SafeString.h ListBase.h
L1AFilTab.o: L1AFilTab.C Filter.h FilterFunc.h EAList.h TlmHdfFile.h ListBase.h CommonDefs.h HdfFile.h Itime.h Parameter.h SafeString.h
L1AParTab.o: L1AParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
L1BHdfFile.o: L1BHdfFile.C L1BHdfFile.h L1AExtract.h ParTab.h TimeTlmFile.h CommonDefs.h Itime.h TlmHdfFile.h Parameter.h HdfFile.h EAList.h SafeString.h ListBase.h
L1BParTab.o: L1BParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
L2AxParTab.o: L2AxParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
L2AParTab.o: L2AParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
L2AhrParTab.o: L2AhrParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
L2BParTab.o: L2BParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h SafeString.h ListBase.h
LeapSecTable.o: EAList.h CommonDefs.h Itime.h LeapSecTable.h LeapSecTable.C
LimitChecker.o: LimitChecker.C LimitChecker.h LimitState.h L1AExtract.h ParTab.h Polynomial.h PolyTable.h Parameter.h Itime.h CommonDefs.h TlmHdfFile.h EAList.h PolyErrNo.h HdfFile.h ListBase.h SafeString.h
LimitList.o: LimitList.C CommonDefs.h LimitList.h ParTab.h PolyTable.h LimitChecker.h LimitState.h L1AExtract.h Parameter.h EAList.h Polynomial.h PolyErrNo.h Itime.h TlmHdfFile.h ListBase.h HdfFile.h SafeString.h
LimitState.o: LimitState.C LimitState.h L1AExtract.h ParTab.h TlmHdfFile.h Parameter.h CommonDefs.h Itime.h HdfFile.h EAList.h SafeString.h ListBase.h
ListBase.o: ListBase.C ListBase.h
Mail.o: Mail.C Mail.h EAConfigList.h EAList.h ListBase.h
MemoryFromFile.o: MemoryFromFile.C MemoryFromFile.h SafeString.h EAList.h CommonDefs.h
MemoryFromTlm.o: MemoryFromTlm.C MemoryFromTlm.h ParTab.h Parameter.h TlmHdfFile.h EAList.h
Qpa.o: Qpa.C Qpf.h Qpa.h CommonDefs.h Itime.h CmdList.h EAList.h Command.h CmdEffect.h Eqx.h Parameter.h ListBase.h Tpg.h
Qpf.o: Qpf.C Qpf.h Itime.h Command.h CmdList.h Binning.h CommonDefs.h Tpg.h EAList.h CmdEffect.h Eqx.h Parameter.h ListBase.h
ParTab.o: ParTab.C Parameter.h ParTab.h Itime.h HdfFile.h CommonDefs.h EAList.h SafeString.h ListBase.h
Parameter.o: Parameter.C Parameter.h CommonDefs.h
ParameterList.o: ParameterList.C ParameterList.h Itime.h ParTab.h PolyTable.h TlmHdfFile.h CommonDefs.h Parameter.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h SafeString.h
	$(CC) -c $(CDEBUGFLAGS) -D__SCAT_GNUCPP__ -DFUNCPROTO -D$(DEBUG) \
	-D_REENTRANT -DXTFUNCPROTO -D__EXTENSIONS__ \
	$(INCFLAGS) -DNOPM ParameterList.C
Polynomial.o: Polynomial.C Polynomial.h CommonDefs.h
PolyTable.o: PolyTable.C CommonDefs.h PolyTable.h EAList.h Polynomial.h PolyErrNo.h ListBase.h
Print.o: Print.C Print.h
Reqa.o: Reqa.C Reqa.h Reqq.h ReqaqList.h Command.h CmdList.h EAList.h Itime.h CommonDefs.h Tpg.h CmdEffect.h Eqx.h Parameter.h ListBase.h
Reqq.o: Reqq.C CommonDefs.h Reqq.h ReqaqList.h Command.h CmdList.h Binning.h Qpf.h EAList.h Itime.h Tpg.h CmdEffect.h Eqx.h Parameter.h ListBase.h
ReqaqList.o: ReqaqList.C ReqaqList.h EAList.h Itime.h Reqq.h ListBase.h CommonDefs.h Command.h CmdList.h Tpg.h CmdEffect.h Eqx.h Parameter.h
Rtcf.o: Rtcf.C CommonDefs.h Command.h Rtcf.h Reqq.h Binning.h Tpg.h CmdList.h Itime.h EAList.h CmdEffect.h Eqx.h Parameter.h ListBase.h
SafeString.o: SafeString.C 
State.o: State.C State.h Parameter.h Itime.h TlmHdfFile.h CommonDefs.h HdfFile.h EAList.h SafeString.h ListBase.h
TimeTlmFile.o: TimeTlmFile.C TimeTlmFile.h CommonDefs.h TlmHdfFile.h Itime.h Parameter.h HdfFile.h EAList.h SafeString.h ListBase.h
TlmHdfFile.o: TlmHdfFile.C TlmHdfFile.h CommonDefs.h HdfFile.h Itime.h Parameter.h EAList.h SafeString.h ListBase.h
TlmFileList.o: TlmFileList.C TlmFileList.h Hk2File.h L1AFile.h NoTimeTlmFile.h EAList.h TlmHdfFile.h Parameter.h Itime.h TimeTlmFile.h ListBase.h CommonDefs.h HdfFile.h SafeString.h
ToDo.o: ToDo.C ToDo.h CommonDefs.h Itime.h EAList.h ListBase.h
Tpg.o: Tpg.C Tpg.h Itime.h CommonDefs.h
UpldTbl.o: UpldTbl.C UpldTbl_1.h Itime.h Command_1.h SafeString.h ArgsPlus.h UpldTbl.h Command.h UpldTblList.h
UpldTblList.o: UpldTblList.C UpldTblList.h EAList.h UpldTbl.h Itime.h CommonDefs.h
UpldTbl.h: UpldTbl_1.h
