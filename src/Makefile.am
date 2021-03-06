EXTRA_DIRS =
if ! WANT_ANDROID
EXTRA_DIRS += APs emacs_mode sql workspaces
endif
SUBDIRS = native $(EXTRA_DIRS)
DIST_SUBDIRS = $(SUBDIRS) testcases

datadir = @datadir@
localedir = $(datadir)/locale

common_sources = \
			buildtag		buildtag.hh		\
						../config.h		\
						APL_types.hh		\
Archive.cc					Archive.hh		\
ArrayIterator.cc				ArrayIterator.hh	\
Assert.cc					Assert.hh		\
Avec.cc			Avec.def		Avec.hh			\
Backtrace.cc					Backtrace.hh		\
Bif_F12_FORMAT.cc				Bif_F12_FORMAT.hh	\
Bif_F12_SORT.cc					Bif_F12_SORT.hh		\
Bif_OPER1_COMMUTE.cc				Bif_OPER1_COMMUTE.hh	\
Bif_OPER1_EACH.cc				Bif_OPER1_EACH.hh	\
Bif_OPER2_POWER.cc				Bif_OPER2_POWER.hh	\
Bif_OPER2_INNER.cc				Bif_OPER2_INNER.hh	\
Bif_OPER2_OUTER.cc				Bif_OPER2_OUTER.hh	\
Bif_OPER2_RANK.cc				Bif_OPER2_RANK.hh	\
Bif_OPER1_REDUCE.cc				Bif_OPER1_REDUCE.hh	\
Bif_OPER1_SCAN.cc				Bif_OPER1_SCAN.hh	\
CDR.cc						CDR.hh			\
Cell.cc						Cell.hh			\
CharCell.cc					CharCell.hh		\
Command.cc		Command.def		Command.hh		\
Common.cc					Common.hh		\
ComplexCell.cc				    	ComplexCell.hh		\
configure_args.cc							\
DiffOut.cc					DiffOut.hh		\
DynamicObject.cc				DynamicObject.hh	\
						EOC_handler_args.hh	\
Error.cc		Error.def		Error.hh		\
						ErrorCode.hh		\
Executable.cc					Executable.hh		\
FloatCell.cc					FloatCell.hh		\
Function.cc					Function.hh		\
DerivedFunction.cc				DerivedFunction.hh	\
						Heapsort.hh		\
Id.cc			Id.def			Id.hh			\
IndexExpr.cc					IndexExpr.hh		\
IndexIterator.cc				IndexIterator.hh	\
InputFile.cc					InputFile.hh		\
IntCell.cc					IntCell.hh		\
LApack.cc					LApack.hh		\
LibPaths.cc					LibPaths.hh		\
LineInput.cc					LineInput.hh		\
Logging.cc	    Logging.def			Logging.hh		\
LvalCell.cc					LvalCell.hh		\
						makefile.h		\
Malloc_hooks.cc								\
Nabla.cc					Nabla.hh		\
NamedObject.cc					NamedObject.hh		\
NativeFunction.cc				NativeFunction.hh	\
						Native_interface.hh	\
NumericCell.cc					NumericCell.hh		\
Output.cc					Output.hh		\
Parser.cc					Parser.hh		\
Prefix.cc		Prefix.def		Prefix.hh		\
PointerCell.cc					PointerCell.hh		\
PrimitiveFunction.cc				PrimitiveFunction.hh	\
PrimitiveOperator.cc				PrimitiveOperator.hh	\
PrintBuffer.cc					PrintBuffer.hh		\
						PrintContext.hh		\
						PrintOperator.hh	\
QuadFunction.cc					QuadFunction.hh		\
ProcessorID.cc					ProcessorID.hh		\
Quad_FX.cc					Quad_FX.hh		\
Quad_RL.cc					Quad_RL.hh		\
Quad_SVx.cc					Quad_SVx.hh		\
Quad_TF.cc					Quad_TF.hh		\
Parallel.cc					Parallel.hh		\
Performance.cc		Performance.def		Performance.hh		\
RealCell.cc					RealCell.hh		\
Shape.cc					Shape.hh		\
						SharedValuePointer.hh	\
						Simple_string.hh	\
ScalarFunction.cc				ScalarFunction.hh	\
						Source.hh		\
StateIndicator.cc				StateIndicator.hh	\
Svar_DB.cc					Svar_DB.hh		\
Svar_record.cc					Svar_record.hh		\
tcp_signal.m4		Svar_signals.def	Svar_signals.hh		\
Symbol.cc					Symbol.hh		\
SymbolTable.cc					SymbolTable.hh		\
			SystemLimits.def	SystemLimits.hh		\
SystemVariable.cc	SystemVariable.def	SystemVariable.hh	\
IO_Files.cc					IO_Files.hh		\
Token.cc		Token.def		Token.hh		\
						TokenEnums.hh		\
Tokenizer.cc					Tokenizer.hh		\
UCS_string.cc					UCS_string.hh		\
						Unicode.hh		\
UserFunction.cc					UserFunction.hh		\
UserPreferences.cc				UserPreferences.hh	\
UTF8_string.cc					UTF8_string.hh		\
Value.cc		Value.icc		Value.hh		\
ValueHistory.cc					ValueHistory.hh		\
Workspace.cc					Workspace.hh




pkglib_LTLIBRARIES =
if WANT_LIBAPL
pkglib_LTLIBRARIES += libapl.la
pkginclude_HEADERS = libapl.h
libapl_la_SOURCES = $(common_sources) libapl.h libapl.cc
libapl_la_LDFLAGS = -avoid-version -module -shared -export-dynamic
else

bin_PROGRAMS = apl
apl_SOURCES = main.cc $(common_sources)

endif


# normally we would say:
#
# Svar_signals.hh: Svar_signals.def tcp_signal.m4
#
# but what if m4 is missing? So we explicitly 'make update-svar_signals'
# when Svar_signals.def or tcp_signal.m4 have changed (which rarely happens)
#
.PHONY:	update-svar_signals
update-svar_signals:
	m4 -D protocol=Svar_signals tcp_signal.m4 > Svar_signals.hh

apl_CXXFLAGS = $(GPROF_WANTED) $(CXX_RDYNAMIC)
libapl_la_CXXFLAGS = $(GPROF_WANTED) $(CXX_RDYNAMIC)

if DEVELOP
   apl_CXXFLAGS       += -Werror -Wall -Wno-strict-aliasing
   libapl_la_CXXFLAGS += -Werror -Wall -Wno-strict-aliasing
endif

apl_LDADD = $(LIBS)

apl.lines: apl
	objdump -j .text -l -d $< > $@

test: all
	./apl --id 1010 -T testcases/*tc

test1: all
	./apl --id 1010 --TM 3 -T testcases/*\.tc

test2: all
	./apl --id 1010 --TR -T testcases/*tc

# run testcases related to APs
testAP: all
	./apl --id 1010 -T testcases/AP*tc

perf: apl
	./apl --id 1010 --TM 3 -T testcases/Performance.pt


AM_MAKEFLAGS = -j 8

makefile.h: Makefile
	@echo "// some strings exported from Makefile"           > $@
	@echo "//"                                              >> $@
	@echo "#define Makefile__bindir     \"${bindir}\""      >> $@
	@echo "#define Makefile__docdir     \"${docdir}\""      >> $@
	@echo "#define Makefile__sysconfdir \"${sysconfdir}\""  >> $@
	@echo "#define Makefile__pkglibdir  \"${pkglibdir}\""   >> $@
	@echo "#define Makefile__localedir  \"${localedir}\""   >> $@
	@echo "#define Makefile__srcdir     \"${abs_srcdir}\""  >> $@
	@echo                                                   >> $@

.PHONY:	patch
patch:
	mkdir -p patches
	svn diff > patches/`date +patch_%F-%R.diff`

.PHONY:	git-pull
git-pull:
	make -C emacs_mode $@
	make -C sql $@

.PHONY:	help
help:
	@echo ""
	@echo ""
	@echo "in addition to standard targets (all, install, clean, ...) "
	@echo "the following make targets may be supported (at this level):"
	@echo ""
	@echo "    make help                    - print this text"
	@echo "    make git-pull                - pull sql and emacs subdirs"
	@echo "    make update-svar_signals     - remake Svar_signals.hh"
	@echo ""

clean:
	rm -f makefile.h

