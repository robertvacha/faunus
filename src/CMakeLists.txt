include_directories(${CMAKE_SOURCE_DIR}/include)

# -------------------------------
#   Faunus library object files
# -------------------------------
set(objs
  geometry potentials slump species analysis)
set_source_files_properties(${objs} PROPERTIES LANGUAGE CXX)

#----- xdrfile static library for gromacs output -----
add_subdirectory(xdrfile)

if(ENABLE_HASHTABLE)
  add_definitions(-DFAU_HASHTABLE)
endif()

if(NOT ENABLE_UNICODE)
  add_definitions(-DAVOID_UNICODE)
endif()
# -------------------------------------
#   Mersenne Twister requires
# ----=--------------------------------
if(ENABLE_TWISTER)
  check_include_file_cxx("random" HAVERANDOM)
  if(HAVERANDOM)
    add_definitions(-DMERSENNETWISTER)
  endif()
endif()

# -------------------------------------
#   Use approximate math funtion?
# -------------------------------------
if(ENABLE_APPROXMATH)
  add_definitions(-DFAU_APPROXMATH)
endif()

# -------------------------------------
#   Get subversion revision of source
# -------------------------------------
if(Subversion_FOUND)
  Subversion_WC_INFO(${CMAKE_HOME_DIRECTORY} Faunus ERROR_QUIET)
  if(Faunus_WC_FOUND)
    add_definitions("-D__SVN_REV__=${Faunus_WC_REVISION}")
  endif()
endif()

# ----------------------------
#  Fetch 3rd-party sasa class
# ----------------------------
if(ENABLE_POWERSASA)
  foreach(powf array.hpp power_diagram.h power_sasa.h LICENSE)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/include/faunus/sasa/${powf}")
      file(DOWNLOAD
        "http://sourceforge.net/p/pteros/code/ci/master/tree/src/core/sasa/${powf}?format=raw"
        "${CMAKE_SOURCE_DIR}/include/faunus/sasa/${powf}" STATUS status)
    endif()
  endforeach()
  add_definitions("-DFAU_POWERSASA")
  include_directories("${CMAKE_SOURCE_DIR}/include/faunus/sasa")
endif()

# -----------------------
#   Link with openbabel
# -----------------------
if(ENABLE_BABEL)
  find_package(OpenBabel2)
  if (OPENBABEL_FOUND)
    set(objs ${objs} iobabel)
    include_directories(${OPENBABEL_INCLUDE_DIR})
    set(LINKLIBS ${LINKLIBS} ${OPENBABEL_LIBRARIES})
    add_definitions(-DBABEL)
  endif()
endif()

# -----------------------
#   Link with MPI
# -----------------------
if(ENABLE_MPI)
  find_package(MPI)
  if(MPI_CXX_FOUND)
    set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_CXX_COMPILE_FLAGS})
    if (APPLE)
      set(CMAKE_CXX_LINK_FLAGS ${MPI_CXX_LINK_FLAGS})
    else()
      set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_CXX_LINK_FLAGS})
    endif()
    set(LINKLIBS ${LINKLIBS} ${MPI_CXX_LIBRARIES})
    include_directories(${MPI_CXX_INCLUDE_PATH})
    add_definitions(-DENABLE_MPI)
  endif()
endif()

# -----------------------------
#   Link with fortran library
# -----------------------------
if(ENABLE_FORTRAN AND CMAKE_Fortran_COMPILER_WORKS)
  set(LINKLIBS ${LINKLIBS} fortranfunc)
endif()

# --------------------
#   Faunus libraries
# --------------------
if (ENABLE_STATIC)
  add_library(libfaunus STATIC ${objs})
else()
  add_library(libfaunus SHARED ${objs})
endif()
set_target_properties(libfaunus PROPERTIES OUTPUT_NAME faunus)
target_link_libraries(libfaunus xdrfile ${LINKLIBS})
export(TARGETS xdrfile libfaunus FILE libfaunus.cmake)
#install(TARGETS libfaunus LIBRARY DESTINATION lib)

# dynamic for cigarparticles
add_library(libcigarfaunus SHARED ${objs})
set_target_properties(libcigarfaunus PROPERTIES
  OUTPUT_NAME cigarfaunus
  COMPILE_DEFINITIONS "CIGARPARTICLE"
  EXCLUDE_FROM_ALL TRUE
  )
target_link_libraries(libcigarfaunus xdrfile ${LINKLIBS})

# dynamic for dipolar particles
add_library(libdipolefaunus SHARED ${objs})
set_target_properties(libdipolefaunus PROPERTIES
  OUTPUT_NAME dipolefaunus
  COMPILE_DEFINITIONS "DIPOLEPARTICLE"
  EXCLUDE_FROM_ALL TRUE
  )
target_link_libraries(libdipolefaunus xdrfile ${LINKLIBS})

# dynamic for hypersphere
add_library(libhyperfaunus SHARED ${objs})
set_target_properties(libhyperfaunus PROPERTIES
  OUTPUT_NAME hyperfaunus
  COMPILE_DEFINITIONS "HYPERSPHERE"
  EXCLUDE_FROM_ALL TRUE
  )
target_link_libraries(libhyperfaunus xdrfile ${LINKLIBS})

#----- header install target -----
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/faunus
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeFiles" EXCLUDE)

# -----------------------------------------
#   Add "cppcheck" static analyzer target
# -----------------------------------------
find_program(CPPCHECK_EXE "cppcheck")
if (CPPCHECK_EXE)
  add_custom_target(
    cppcheck
    COMMAND ${CPPCHECK_EXE}
    -I ${CMAKE_SOURCE_DIR}/include -j2 --quiet --enable=style
    ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/moves/*.cpp
    COMMENT "Running C++ static analyzer..."
    )
  set_target_properties(cppcheck PROPERTIES EXCLUDE_FROM_ALL TRUE )
endif()

# -----------------------------------------
#   Function to add a generic C++ example
# -----------------------------------------
function( fau_example tname tdir tsrc )
  add_executable( ${tname} "${tdir}/${tsrc}" )
  set_source_files_properties( "${tdir}/${tsrc}" PROPERTIES LANGUAGE CXX)
  set_target_properties(${tname}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${tdir}/")
  target_link_libraries(${tname} libfaunus)
  install (DIRECTORY "${tdir}"
    DESTINATION "share/faunus/examples"
    PATTERN ".svn" EXCLUDE)
endfunction( fau_example )


#----- Add example programs -----
add_subdirectory(tools)
add_subdirectory(playground EXCLUDE_FROM_ALL)
add_subdirectory(examples)

