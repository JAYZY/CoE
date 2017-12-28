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
	${OBJECTDIR}/CLAUSE/Clause.o \
	${OBJECTDIR}/CLAUSE/Formula.o \
	${OBJECTDIR}/CLAUSE/Literal.o \
	${OBJECTDIR}/Global/Environment.o \
	${OBJECTDIR}/Global/IncDefine.o \
	${OBJECTDIR}/HEURISTICS/Options.o \
	${OBJECTDIR}/INOUT/FileOp.o \
	${OBJECTDIR}/INOUT/Scanner.o \
	${OBJECTDIR}/INOUT/StreamCell.o \
	${OBJECTDIR}/INOUT/TokenCell.o \
	${OBJECTDIR}/LIB/Out.o \
	${OBJECTDIR}/Prover/Prover.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath,'.'

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/coprover

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/coprover: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/coprover ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CLAUSE/Clause.o: CLAUSE/Clause.cpp
	${MKDIR} -p ${OBJECTDIR}/CLAUSE
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CLAUSE/Clause.o CLAUSE/Clause.cpp

${OBJECTDIR}/CLAUSE/Formula.o: CLAUSE/Formula.cpp
	${MKDIR} -p ${OBJECTDIR}/CLAUSE
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CLAUSE/Formula.o CLAUSE/Formula.cpp

${OBJECTDIR}/CLAUSE/Literal.o: CLAUSE/Literal.cpp
	${MKDIR} -p ${OBJECTDIR}/CLAUSE
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CLAUSE/Literal.o CLAUSE/Literal.cpp

${OBJECTDIR}/Global/Environment.o: Global/Environment.cpp
	${MKDIR} -p ${OBJECTDIR}/Global
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Global/Environment.o Global/Environment.cpp

${OBJECTDIR}/Global/IncDefine.o: Global/IncDefine.cpp
	${MKDIR} -p ${OBJECTDIR}/Global
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Global/IncDefine.o Global/IncDefine.cpp

${OBJECTDIR}/HEURISTICS/Options.o: HEURISTICS/Options.cpp
	${MKDIR} -p ${OBJECTDIR}/HEURISTICS
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HEURISTICS/Options.o HEURISTICS/Options.cpp

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

${OBJECTDIR}/LIB/Out.o: LIB/Out.cpp
	${MKDIR} -p ${OBJECTDIR}/LIB
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LIB/Out.o LIB/Out.cpp

${OBJECTDIR}/Prover/Prover.o: Prover/Prover.cpp
	${MKDIR} -p ${OBJECTDIR}/Prover
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Prover/Prover.o Prover/Prover.cpp

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
