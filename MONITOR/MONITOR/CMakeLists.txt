# **************************************************************************
# * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
# *                                                                        *
# * Author: The ALICE Off-line Project.                                    *
# * Contributors are mentioned in the code where appropriate.              *
# *                                                                        *
# * Permission to use, copy, modify and distribute this software and its   *
# * documentation strictly for non-commercial purposes is hereby granted   *
# * without fee, provided that the above copyright notice appears in all   *
# * copies and that both the copyright notice and this permission notice   *
# * appear in the supporting documentation. The authors make no claims     *
# * about the suitability of this software for any purpose. It is          *
# * provided "as is" without express or implied warranty.                  *
# **************************************************************************

# Module
set(MODULE MONITOR)

# Module include folder
include_directories(${AliRoot_SOURCE_DIR}/MONITOR/${MODULE}
                    ${AliRoot_SOURCE_DIR}/MONITOR/alistoragemanager
                   )

# Additional include folders in alphabetical order
include_directories(${AliRoot_SOURCE_DIR}/STEER/STEERBase
                   )

# Sources in alphabetical order
set(SRCS
    AliChildProcTerminator.cxx
    AliDimIntNotifier.cxx
    AliOnlineRecoTrigger.cxx
    AliQAHistNavigator.cxx
    AliQAHistViewer.cxx
    AliTestChildProc.cxx
   )

if(DIMDIR AND ODIR)
    set(SRCS ${SRCS}
        SORNotifier.cxx
       )
    include_directories(${DIMDIR}/dim)
    link_directories(${DIMDIR}/${ODIR})
    add_definitions(-DALI_DIM)
endif()

# Headers from sources
string(REPLACE ".cxx" ".h" HDRS "${SRCS}")

# Generate the dictionary
# It will create G_ARG1.cxx and G_ARG1.h / ARG1 = function first argument
get_directory_property(incdirs INCLUDE_DIRECTORIES)
generate_dictionary("${MODULE}" "${MODULE}LinkDef.h" "${HDRS}" "${incdirs}")

# Generate the ROOT map
# Dependecies
set(LIBDEPS STEERBase Core Gpad Graf Gui Hist RIO Thread)
generate_rootmap("${MODULE}" "${LIBDEPS}" "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE}LinkDef.h")

# Add a library to the project using the specified source files
add_library(${MODULE} SHARED ${SRCS} G__${MODULE}.cxx)
target_link_libraries(${MODULE} STEERBase Core Gpad Graf Gui Hist RIO Thread)
if(DIMDIR AND ODIR)
    target_link_libraries(${MODULE} dim)
endif()

# Additional compilation flags
set_target_properties(${MODULE} PROPERTIES COMPILE_FLAGS "-O -g")

# System dependent: Modify the way the library is build
if(${CMAKE_SYSTEM} MATCHES Darwin)
    set_target_properties(${MODULE} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(${CMAKE_SYSTEM} MATCHES Darwin)

# Installation
install(TARGETS ${MODULE}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

install(FILES ${HDRS} DESTINATION include)