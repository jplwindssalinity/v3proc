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

%.C: $(SIM_CENTRAL_TREE)/src/eadata/RCS/%.C,v
	co $(SIM_CENTRAL_TREE)/src/eadata/RCS/$@,v
 
%.h: $(SIM_CENTRAL_TREE)/src/eadata/RCS/%.h,v
	co $(SIM_CENTRAL_TREE)/src/eadata/RCS/$@,v

Args.o: Args.C Args.h ArgDefs.h EAConfigList.h EALog.h EAList.h TlmFileList.h TlmHdfFile.h ListBase.h Parameter.h Itime.h CommonDefs.h HdfFile.h
ArgsPlus.o: ArgsPlus.C Args.h ArgsPlus.h ParTab.h EAConfigList.h EALog.h TlmFileList.h LimitList.h Filter.h ToDo.h CmdList.h Eqx.h PolyTable.h ArgDefs.h Itime.h Parameter.h CommonDefs.h EAList.h TlmHdfFile.h LimitChecker.h Command.h Tpg.h Polynomial.h PolyErrNo.h ListBase.h HdfFile.h LimitState.h
Backup.o: Backup.C Backup.h
Binning.o: Binning.C Binning.h NSCAT.h
CmdList.o: CmdList.C Command.h CmdList.h CmdState.h Eqx.h Itime.h CommonDefs.h Tpg.h EAList.h Parameter.h ListBase.h
CmdState.o: CmdState.C CmdState.h Command.h Parameter.h CommonDefs.h Tpg.h Itime.h
Command.o: Command.C Command.h Tpg.h CommonDefs.h Itime.h
CommonDefs.o: CommonDefs.C Parameter.h CommonDefs.h
Drn.o: Drn.C Drn.h Itime.h Mail.h EAConfigList.h CommonDefs.h EAList.h ListBase.h
DrvParamList.o: DrvParamList.C DrvParamList.h Itime.h ParTab.h PolyTable.h TlmHdfFile.h ParameterList.h CommonDefs.h Parameter.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h
EAConfigList.o: EAConfigList.C CommonDefs.h EAConfigList.h EAList.h ListBase.h
EAList.o: EAList.C EAList.h ListBase.h
Eqx.o: Eqx.C Eqx.h Itime.h EAList.h Tpg.h CommonDefs.h ListBase.h
Filter.o: Filter.C Filter.h ParTab.h EAList.h TlmHdfFile.h CommonDefs.h Parameter.h ListBase.h HdfFile.h Itime.h
HdfFile.o: HdfFile.C HdfFile.h EAList.h ListBase.h
Headers.o: Headers.C Headers.h Parameter.h Itime.h CommonDefs.h
Itime.o: Itime.C Itime.h CommonDefs.h
L1AExtract.o: L1AExtract.C L1AExtract.h Parameter.h Itime.h CommonDefs.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
L1AFilTab.o: L1AFilTab.C Filter.h L1AFilter.h EAList.h TlmHdfFile.h ListBase.h CommonDefs.h HdfFile.h Itime.h Parameter.h
L1AFile.o: L1AFile.C L1AFile.h L1AExtract.h ParTab.h TlmHdfFile.h Itime.h Parameter.h CommonDefs.h HdfFile.h EAList.h ListBase.h
L1AFilter.o: L1AFilter.C Filter.h L1AExtract.h Parameter.h ParTab.h EAList.h TlmHdfFile.h CommonDefs.h Itime.h ListBase.h HdfFile.h
L1AParTab.o: L1AParTab.C ParTab.h L1AExtract.h Print.h CommonDefs.h Parameter.h Itime.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
LimitChecker.o: LimitChecker.C LimitChecker.h LimitState.h HkdtExtract.h L1AExtract.h ParTab.h Parameter.h Itime.h CommonDefs.h TlmHdfFile.h HdfFile.h EAList.h ListBase.h
LimitList.o: LimitList.C CommonDefs.h LimitList.h ParTab.h LimitChecker.h LimitState.h HkdtExtract.h L1AExtract.h Parameter.h EAList.h Itime.h TlmHdfFile.h ListBase.h HdfFile.h
LimitState.o: LimitState.C LimitState.h HkdtExtract.h L1AExtract.h ParTab.h TlmHdfFile.h Parameter.h CommonDefs.h Itime.h HdfFile.h EAList.h ListBase.h
Mail.o: Mail.C Mail.h EAConfigList.h EAList.h ListBase.h
ParTab.o: ParTab.C Parameter.h ParTab.h Itime.h HdfFile.h CommonDefs.h EAList.h ListBase.h
Parameter.o: Parameter.C Parameter.h CommonDefs.h
ParameterList.o: ParameterList.C ParameterList.h Itime.h ParTab.h PolyTable.h TlmHdfFile.h CommonDefs.h Parameter.h EAList.h Polynomial.h PolyErrNo.h HdfFile.h ListBase.h
Print.o: Print.C Print.h
Reqq.o: Reqq.C CommonDefs.h Reqq.h Command.h CmdList.h Binning.h Tpg.h EAList.h Eqx.h Itime.h Parameter.h ListBase.h
Rtcf.o: Rtcf.C CommonDefs.h Command.h Rtcf.h Reqq.h Binning.h Tpg.h CmdList.h Itime.h EAList.h Eqx.h Parameter.h ListBase.h
TempInst.o: TempInst.C EAList.h EAList.C Command.h EAConfigList.h Eqx.h Filter.h LimitChecker.h Parameter.h Polynomial.h TlmHdfFile.h ToDo.h ListBase.h CommonDefs.h Tpg.h Itime.h LimitState.h HdfFile.h
TlmFileList.o: TlmFileList.C TlmFileList.h L1AFile.h NoTimeTlmFile.h EAList.h TlmHdfFile.h Parameter.h Itime.h ListBase.h CommonDefs.h HdfFile.h
TlmHdfFile.o: TlmHdfFile.C TlmHdfFile.h CommonDefs.h HdfFile.h Itime.h Parameter.h EAList.h ListBase.h
ToDo.o: ToDo.C ToDo.h CommonDefs.h Itime.h EAList.h ListBase.h
Tpg.o: Tpg.C Tpg.h Itime.h CommonDefs.h
