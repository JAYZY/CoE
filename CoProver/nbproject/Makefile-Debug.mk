#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Alg/Resolution.o \
	${OBJECTDIR}/Alg/TriAlg.o \
	${OBJECTDIR}/CLAUSE/Clause.o \
	${OBJECTDIR}/CLAUSE/Literal.o \
	${OBJECTDIR}/Formula/ClauseSet.o \
	${OBJECTDIR}/Formula/Formula.o \
	${OBJECTDIR}/Formula/FormulaSet.o \
	${OBJECTDIR}/Formula/WFormula.o \
	${OBJECTDIR}/Global/Environment.o \
	${OBJECTDIR}/Global/IncDefine.o \
	${OBJECTDIR}/Global/SysDate.o \
	${OBJECTDIR}/HEURISTICS/Options.o \
	${OBJECTDIR}/HEURISTICS/SortRule.o \
	${OBJECTDIR}/HEURISTICS/StrategyParam.o \
	${OBJECTDIR}/INOUT/CommandLine.o \
	${OBJECTDIR}/INOUT/FileOp.o \
	${OBJECTDIR}/INOUT/Scanner.o \
	${OBJECTDIR}/INOUT/StreamCell.o \
	${OBJECTDIR}/INOUT/TokenCell.o \
	${OBJECTDIR}/Indexing/TermIndexing.o \
	${OBJECTDIR}/Inferences/InferenceInfo.o \
	${OBJECTDIR}/Inferences/Simplification.o \
	${OBJECTDIR}/Inferences/Subst.o \
	${OBJECTDIR}/Inferences/Unify.o \
	${OBJECTDIR}/LIB/Out.o \
	${OBJECTDIR}/Orderings/KBO.o \
	${OBJECTDIR}/Orderings/Ordering.o \
	${OBJECTDIR}/PROOF/ProofControl.o \
	${OBJECTDIR}/PROOF/ProverResultAnalyse.o \
	${OBJECTDIR}/Prover/Prover.o \
	${OBJECTDIR}/Terms/GroundTermBank.o \
	${OBJECTDIR}/Terms/Sigcell.o \
	${OBJECTDIR}/Terms/TermBank.o \
	${OBJECTDIR}/Terms/TermCell.o \
	${OBJECTDIR}/Terms/TermCellStore.o \
	${OBJECTDIR}/Terms/TermTree.o \
	${OBJECTDIR}/Terms/VarBank.o \
	${OBJECTDIR}/Terms/VarHash.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m64
CXXFLAGS=-m64

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L. -Wl,-rpath,'.'

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/coprover

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/coprover: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/coprover ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Alg/Resolution.o: Alg/Resolution.cpp
	${MKDIR} -p ${OBJECTDIR}/Alg
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Alg/Resolution.o Alg/Resolution.cpp

${OBJECTDIR}/Alg/TriAlg.o: Alg/TriAlg.cpp
	${MKDIR} -p ${OBJECTDIR}/Alg
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Alg/TriAlg.o Alg/TriAlg.cpp

${OBJECTDIR}/CLAUSE/Clause.o: CLAUSE/Clause.cpp
	${MKDIR} -p ${OBJECTDIR}/CLAUSE
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CLAUSE/Clause.o CLAUSE/Clause.cpp

${OBJECTDIR}/CLAUSE/Literal.o: CLAUSE/Literal.cpp
	${MKDIR} -p ${OBJECTDIR}/CLAUSE
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CLAUSE/Literal.o CLAUSE/Literal.cpp

${OBJECTDIR}/Formula/ClauseSet.o: Formula/ClauseSet.cpp
	${MKDIR} -p ${OBJECTDIR}/Formula
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Formula/ClauseSet.o Formula/ClauseSet.cpp

${OBJECTDIR}/Formula/Formula.o: Formula/Formula.cpp
	${MKDIR} -p ${OBJECTDIR}/Formula
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Formula/Formula.o Formula/Formula.cpp

${OBJECTDIR}/Formula/FormulaSet.o: Formula/FormulaSet.cpp
	${MKDIR} -p ${OBJECTDIR}/Formula
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Formula/FormulaSet.o Formula/FormulaSet.cpp

${OBJECTDIR}/Formula/WFormula.o: Formula/WFormula.cpp
	${MKDIR} -p ${OBJECTDIR}/Formula
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Formula/WFormula.o Formula/WFormula.cpp

${OBJECTDIR}/Global/Environment.o: Global/Environment.cpp
	${MKDIR} -p ${OBJECTDIR}/Global
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Global/Environment.o Global/Environment.cpp

${OBJECTDIR}/Global/IncDefine.o: Global/IncDefine.cpp
	${MKDIR} -p ${OBJECTDIR}/Global
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Global/IncDefine.o Global/IncDefine.cpp

${OBJECTDIR}/Global/SysDate.o: Global/SysDate.cpp
	${MKDIR} -p ${OBJECTDIR}/Global
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Global/SysDate.o Global/SysDate.cpp

${OBJECTDIR}/HEURISTICS/Options.o: HEURISTICS/Options.cpp
	${MKDIR} -p ${OBJECTDIR}/HEURISTICS
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HEURISTICS/Options.o HEURISTICS/Options.cpp

${OBJECTDIR}/HEURISTICS/SortRule.o: HEURISTICS/SortRule.cpp
	${MKDIR} -p ${OBJECTDIR}/HEURISTICS
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HEURISTICS/SortRule.o HEURISTICS/SortRule.cpp

${OBJECTDIR}/HEURISTICS/StrategyParam.o: HEURISTICS/StrategyParam.cpp
	${MKDIR} -p ${OBJECTDIR}/HEURISTICS
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HEURISTICS/StrategyParam.o HEURISTICS/StrategyParam.cpp

${OBJECTDIR}/INOUT/CommandLine.o: INOUT/CommandLine.cpp
	${MKDIR} -p ${OBJECTDIR}/INOUT
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/INOUT/CommandLine.o INOUT/CommandLine.cpp

${OBJECTDIR}/INOUT/FileOp.o: INOUT/FileOp.cpp
	${MKDIR} -p ${OBJECTDIR}/INOUT
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/INOUT/FileOp.o INOUT/FileOp.cpp

${OBJECTDIR}/INOUT/Scanner.o: INOUT/Scanner.cpp
	${MKDIR} -p ${OBJECTDIR}/INOUT
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/INOUT/Scanner.o INOUT/Scanner.cpp

${OBJECTDIR}/INOUT/StreamCell.o: INOUT/StreamCell.cpp
	${MKDIR} -p ${OBJECTDIR}/INOUT
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/INOUT/StreamCell.o INOUT/StreamCell.cpp

${OBJECTDIR}/INOUT/TokenCell.o: INOUT/TokenCell.cpp
	${MKDIR} -p ${OBJECTDIR}/INOUT
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/INOUT/TokenCell.o INOUT/TokenCell.cpp

${OBJECTDIR}/Indexing/TermIndexing.o: Indexing/TermIndexing.cpp
	${MKDIR} -p ${OBJECTDIR}/Indexing
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Indexing/TermIndexing.o Indexing/TermIndexing.cpp

${OBJECTDIR}/Inferences/InferenceInfo.o: Inferences/InferenceInfo.cpp
	${MKDIR} -p ${OBJECTDIR}/Inferences
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Inferences/InferenceInfo.o Inferences/InferenceInfo.cpp

${OBJECTDIR}/Inferences/Simplification.o: Inferences/Simplification.cpp
	${MKDIR} -p ${OBJECTDIR}/Inferences
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Inferences/Simplification.o Inferences/Simplification.cpp

${OBJECTDIR}/Inferences/Subst.o: Inferences/Subst.cpp
	${MKDIR} -p ${OBJECTDIR}/Inferences
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Inferences/Subst.o Inferences/Subst.cpp

${OBJECTDIR}/Inferences/Unify.o: Inferences/Unify.cpp
	${MKDIR} -p ${OBJECTDIR}/Inferences
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Inferences/Unify.o Inferences/Unify.cpp

${OBJECTDIR}/LIB/Out.o: LIB/Out.cpp
	${MKDIR} -p ${OBJECTDIR}/LIB
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LIB/Out.o LIB/Out.cpp

${OBJECTDIR}/Orderings/KBO.o: Orderings/KBO.cpp
	${MKDIR} -p ${OBJECTDIR}/Orderings
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Orderings/KBO.o Orderings/KBO.cpp

${OBJECTDIR}/Orderings/Ordering.o: Orderings/Ordering.cpp
	${MKDIR} -p ${OBJECTDIR}/Orderings
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Orderings/Ordering.o Orderings/Ordering.cpp

${OBJECTDIR}/PROOF/ProofControl.o: PROOF/ProofControl.cpp
	${MKDIR} -p ${OBJECTDIR}/PROOF
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PROOF/ProofControl.o PROOF/ProofControl.cpp

${OBJECTDIR}/PROOF/ProverResultAnalyse.o: PROOF/ProverResultAnalyse.cpp
	${MKDIR} -p ${OBJECTDIR}/PROOF
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PROOF/ProverResultAnalyse.o PROOF/ProverResultAnalyse.cpp

${OBJECTDIR}/Prover/Prover.o: Prover/Prover.cpp
	${MKDIR} -p ${OBJECTDIR}/Prover
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Prover/Prover.o Prover/Prover.cpp

${OBJECTDIR}/Terms/GroundTermBank.o: Terms/GroundTermBank.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/GroundTermBank.o Terms/GroundTermBank.cpp

${OBJECTDIR}/Terms/Sigcell.o: Terms/Sigcell.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/Sigcell.o Terms/Sigcell.cpp

${OBJECTDIR}/Terms/TermBank.o: Terms/TermBank.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/TermBank.o Terms/TermBank.cpp

${OBJECTDIR}/Terms/TermCell.o: Terms/TermCell.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/TermCell.o Terms/TermCell.cpp

${OBJECTDIR}/Terms/TermCellStore.o: Terms/TermCellStore.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/TermCellStore.o Terms/TermCellStore.cpp

${OBJECTDIR}/Terms/TermTree.o: Terms/TermTree.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/TermTree.o Terms/TermTree.cpp

${OBJECTDIR}/Terms/VarBank.o: Terms/VarBank.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/VarBank.o Terms/VarBank.cpp

${OBJECTDIR}/Terms/VarHash.o: Terms/VarHash.cpp
	${MKDIR} -p ${OBJECTDIR}/Terms
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Terms/VarHash.o Terms/VarHash.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
