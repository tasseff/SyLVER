cmake_minimum_required (VERSION 2.6)
include(CheckFunctionExists)
include(CheckFortranFunctionExists)
project (SyLVER)
enable_language(Fortran)
enable_language(C)
enable_language(CXX)

# CMake version
message(STATUS "CMake version: ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}")

# Include our own cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Load CMake helpers
include(cmake/build_helpers.cmake)
include(cmake/install_helpers.cmake)

################################################################################
# Linker options
set(LINKER_LANGUAGE "Fortran")
set(CMAKE_Fortran_LINKER_PREFERENCE 50)

################################################################################
# Compiler options

########################################
# C flags
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2" )
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -ffpe-trap=underflow,denormal" )
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

########################################
# CXX flags
# STD C++11
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# list(APPEND CMAKE_CXX_FLAGS " -std=c++11")
# Note: Not supported feature for Intel compilers in CMake version < 3.6
set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

# Optimization flags
# enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_RELEASE "-tet")
# message("CXX_COMPILER_FLAG_SUPPORTED: ${CXX_COMPILER_FLAG_SUPPORTED}")
# message("CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
# enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-tet")
# message("CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
# enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_MINSIZEREL "-tet")
# message("CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")

# message("CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
# message("CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")

enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_RELEASE "-march=native")
enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-march=native")
enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_MINSIZEREL "-march=native")

if(NOT SYLVER_HAVE_CXX_FLAG_march=native)
  # This is useful on IBM machines where march=native is not
  # necessarily suported
  enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_RELEASE "-mcpu=native")
  enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-mcpu=native")
  enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_MINSIZEREL "-mcpu=native")
endif()

# message("CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
# message("CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
# message("CMAKE_CXX_FLAGS_MINSIZEREL: ${CMAKE_CXX_FLAGS_MINSIZEREL}")

# Debug flags
# set(CMAKE_XCODE_GENERATE_SCHEME ON)
# set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER ON)
# set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=thread")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=bounds -fbounds-check")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fbounds-check")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ffpe-trap=underflow,denormal" )

########################################
# Fortran flags

enable_fortran_compiler_flag_if_supported(CMAKE_Fortran_FLAGS_RELEASE "-march=native")
enable_fortran_compiler_flag_if_supported(CMAKE_Fortran_FLAGS_RELWITHDEBINFO "-march=native")
enable_fortran_compiler_flag_if_supported(CMAKE_Fortran_FLAGS_MINSIZEREL "-march=native")

if(NOT SYLVER_HAVE_Fortran_FLAG_march=native)
  # This is useful on IBM machines where march=native is not
  # necessarily suported
  enable_fortran_compiler_flag_if_supported(CMAKE_Fortran_FLAGS_RELEASE "-mcpu=native")
  enable_fortran_compiler_flag_if_supported(CMAKE_Fortran_FLAGS_RELWITHDEBINFO "-mcpu=native")
  enable_fortran_compiler_flag_if_supported(CMAKE_Fortran_FLAGS_MINSIZEREL "-mcpu=native")
endif()

# IBM machine
# set(CMAKE_Fortran_FLAGS_RELWITHDEBINFO "${CMAKE_Fortran_FLAGS_RELWITHDEBINFO} -mcpu=native")
# Debug flags
# set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -fsanitize=address")
# set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -fsanitize=undefined")
# set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -fsanitize=thread")
set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -Wall -Wextra -Warray-temporaries -Wconversion -fbacktrace -ffree-line-length-0")
set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -ffpe-trap=zero,overflow,underflow")
set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -fcheck=bounds,do,mem,pointer")
set(CMAKE_Fortran_FLAGS_DEBUG "${CMAKE_Fortran_FLAGS_DEBUG} -finit-real=nan")

########################################
# Set default build type
set(default_build_type "RelWithDebInfo")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Using default build type '${default_build_type}' because none was specified")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
    STRING "Choose the type of build (Debug, Release, MinSizeRel, RelWithDebInfo)" FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_Fortran_FLAGS_ALL ${CMAKE_Fortran_FLAGS})
set(CMAKE_C_FLAGS_ALL ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_ALL ${CMAKE_CXX_FLAGS})

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  string(APPEND CMAKE_Fortran_FLAGS_ALL " ${CMAKE_Fortran_FLAGS_DEBUG}")
  string(APPEND CMAKE_C_FLAGS_ALL " ${CMAKE_C_FLAGS_DEBUG}")
  string(APPEND CMAKE_CXX_FLAGS_ALL " ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
  string(APPEND CMAKE_Fortran_FLAGS_ALL " ${CMAKE_Fortran_FLAGS_RELEASE}")
  string(APPEND CMAKE_C_FLAGS_ALL " ${CMAKE_C_FLAGS_RELEASE}")
  string(APPEND CMAKE_CXX_FLAGS_ALL " ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
  string(APPEND CMAKE_Fortran_FLAGS_ALL " ${CMAKE_Fortran_FLAGS_MINSIZEREL}")
  string(APPEND CMAKE_C_FLAGS_ALL " ${CMAKE_C_FLAGS_MINSIZEREL}")
  string(APPEND CMAKE_CXX_FLAGS_ALL " ${CMAKE_CXX_FLAGS_MINSIZEREL}")
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
  string(APPEND CMAKE_Fortran_FLAGS_ALL " ${CMAKE_Fortran_FLAGS_RELWITHDEBINFO}")
  string(APPEND CMAKE_C_FLAGS_ALL " ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
  string(APPEND CMAKE_CXX_FLAGS_ALL " ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

################################################################################
# Options

## Set runtime system to be used for the execution
set(SYLVER_RUNTIME "StarPU" CACHE STRING "Runtime system")
set_property(CACHE SYLVER_RUNTIME PROPERTY STRINGS STF StarPU)
# # Set BLAS and LAPACK libraries to use for linking
# SET(BLAS_LAPACK "MKL" CACHE STRING "BLAS and LAPACK libraries")
# set_property(CACHE BLAS_LAPACK PROPERTY STRINGS Ref MKL)
# MESSAGE( STATUS "BLAS/LAPACK:         " ${BLAS_LAPACK} )

# Enable CUDA
option(SYLVER_ENABLE_CUDA "Enable CUDA" OFF)
# Enable OpenMP
option(SYLVER_ENABLE_OMP "Use OpenMP" OFF)
# Use internal SPRAL
option(SYLVER_SPRAL_USE_INTERNAL "Use OpenMP" ON)
# Build SyLVER examples
option(SYLVER_BUILD_EXAMPLES "Build examples" OFF)
# Build SyLVER tests
option(SYLVER_BUILD_TESTS "Build tests" OFF)
# Build SyLVER drivers
option(SYLVER_BUILD_DRIVERS "Build drivers" OFF)
# Enable profiling
option(SYLVER_ENABLE_PROFILING "Activate profiling info" OFF)
# Enable NVIDIA cutlass library 
option(SYLVER_ENABLE_CUTLASS "CUTLASS library available" OFF)

# Have la-hp scheduler
# option(HAVE_LAHP "LA-heteroprio scheduler available" OFF)
# if (${HAVE_LAHP} MATCHES "ON")
#   add_definitions(-DHAVE_LAHP)
# endif()
# MESSAGE(STATUS "LAHP: " ${HAVE_LAHP})

if (SYLVER_ENABLE_PROFILING)
  add_definitions(-DSPLDLT_USE_PROFILING)
  add_definitions(-DSYLVER_ENABLE_PROFILING)
endif()


################################################################################
# Low-level options

SET(MEMLAYOUT "1D" CACHE STRING "Memory layout")
set_property(CACHE MEMLAYOUT PROPERTY STRINGS 2D 1D)
MESSAGE( STATUS "Memory layout: " ${MEMLAYOUT})
if (${MEMLAYOUT} MATCHES "1D")
  add_definitions(-DMEMLAYOUT_1D)
elseif (${MEMLAYOUT} MATCHES "2D")
  add_definitions(-DMEMLAYOUT_2D)
endif()

################################################################################
# Third-party libraries


########################################
# cutlass

if (SYLVER_ENABLE_CUTLASS)
  set(CUTLASS_DIR "" CACHE STRING "CUTLASS library")

  if(CUTLASS_DIR)
    set(CUTLASS_HEADERS ${CUTLASS_DIR})
  elseif(DEFINED ENV{CUTLASS_DIR})
    set(CUTLASS_HEADERS $ENV{CUTLASS_DIR})
  elseif(DEFINED ENV{CUTLASSDIR})
    set(CUTLASS_HEADERS $ENV{CUTLASSDIR})
  endif()

  if(EXISTS ${CUTLASS_HEADERS})
    message(STATUS "CUTLASS: " ${HAVE_CUTLASS})
    message(STATUS "CUTLASS Directory: " ${CUTLASS_HEADERS})
  else()
    message(FATAL_ERROR "CUTLASS NOT found")
  endif()
  
  add_definitions(-DHAVE_CUTLASS)
endif()


########################################
# HWLOC

find_package(HWLOC)
if (HWLOC_FOUND)
  include_directories(${HWLOC_INCLUDE_DIRS})
  link_directories(${HWLOC_LIBRARY_DIRS})
  link_libraries(${HWLOC_LIBRARIES})
  set(LIBS ${LIBS} ${HWLOC_LIBRARIES})
  # message(STATUS "HWLOC libraries: ${HWLOC_LIBRARIES}")
else()
  message(STATUS "HWLOC library cannot be found during the compilation of SyLVER. You can provide the path of the HWLOC library using either the -DHWLOC_DIR option or the HWLOC_DIR environment variable")
endif()

########################################
# Metis

find_package(METIS)
if (METIS_FOUND)
  # include_directories( ${METIS_INCLUDE_DIRS} )
  link_directories( ${METIS_LIBRARY_DIRS} )
  link_libraries( ${METIS_LIBRARIES} )
  set(LIBS ${LIBS} ${METIS_LIBRARIES})
else()
  message(STATUS "METIS library cannot be found during the compilation of SyLVER. You can provide the path of the METIS library using either the -DMETIS_DIR option or the METIS_DIR environment variable")
endif()
  
########################################
# std C++ (needed for linking with ssids)
set(LIBS ${LIBS} stdc++)

########################################
# BLAS
set(LBLAS "" CACHE STRING "BLAS library")

# If LBLAS not set, use environement variable
if(LBLAS)
  set(BLAS_LIBRARIES ${LBLAS})
elseif(DEFINED ENV{BLAS_LIB})
  set(BLAS_LIBRARIES $ENV{BLAS_LIB})
endif()

if(DEFINED BLAS_LIBRARIES)
  # message(STATUS "BLAS_LIBRARIES: " ${BLAS_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES ${BLAS_LIBRARIES})
  CHECK_FORTRAN_FUNCTION_EXISTS("dgemm" BLAS_FOUND)
  unset(CMAKE_REQUIRED_LIBRARIES)
  if(NOT BLAS_FOUND)
    message(ERROR "User supplied BLAS is NOT working")
  endif()
else()
  find_package(BLAS)
endif()

if(BLAS_FOUND)
  set(LIBS ${LIBS} ${BLAS_LIBRARIES})
else()
  message(FATAL_ERROR "BLAS NOT found")
endif (BLAS_FOUND)

########################################
# LAPACK
set(LLAPACK "" CACHE STRING "LAPACK library")
# If LBLAS not set, use environement variable 
if(LLAPACK)
  set(LAPACK_LIBRARIES ${LLAPACK})
elseif(DEFINED ENV{LAPACK_LIB})
  set(LAPACK_LIBRARIES $ENV{LAPACK_LIB})
endif()
if(DEFINED LAPACK_LIBRARIES)
  set(CMAKE_REQUIRED_LIBRARIES ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
  CHECK_FORTRAN_FUNCTION_EXISTS("dpotrf" LAPACK_FOUND)
  unset(CMAKE_REQUIRED_LIBRARIES)
  if(NOT LAPACK_FOUND)
    message(ERROR "User supplied LAPACK is NOT working")
  endif()

else()
  find_package(LAPACK)
endif()

if(LAPACK_FOUND)
  set(LIBS ${LIBS} ${LAPACK_LIBRARIES} ${BLAS_LIBRARIES})
else()
  message(FATAL_ERROR "LAPACK library NOT found")
endif(LAPACK_FOUND)

########################################
# OpenMP
if(SYLVER_ENABLE_OMP)
  find_package(OpenMP)
  if (OPENMP_FOUND)
    message(STATUS "OpenMP CXX Flags: " ${OpenMP_Fortran_FLAGS})
    # Linker flags
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_Fortran_FLAGS}")
    # set(LIBS ${LIBS} ${OpenMP_CXX_LIBRARIES})

    # Compiler flags
    string(APPEND CMAKE_Fortran_FLAGS_ALL " ${OpenMP_Fortran_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS_ALL " ${OpenMP_Fortran_FLAGS}")
    
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${OpenMP_Fortran_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_Fortran_FLAGS}")

    add_definitions(-DSPLDLT_USE_OMP)
    add_definitions(-DSYLVER_HAVE_OMP)

  else()
    message(FATAL_ERROR "OpenMP library NOT found")
  endif()
endif()

########################################
# CUDA
if(SYLVER_ENABLE_CUDA)

  # Add definitions
  add_definitions(-DSPLDLT_USE_GPU)
  add_definitions(-DSYLVER_HAVE_CUDA)

  # Use CUDA as a native language within CMake
  set(SYLVER_NATIVE_CUDA ON CACHE BOOL "Utilize the CMake native CUDA flow")
  mark_as_advanced(SYLVER_NATIVE_CUDA)

  set(SyLVER_NVCC_ARCHS "60;61;70;75" CACHE STRING "The SM architectures to build code for.")

  if(SYLVER_NATIVE_CUDA)

    enable_language(CUDA)

    # Find cudart library
    find_library(CUDA_RUNTIME_LIBS_DYNAMIC cudart
      HINT ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
    # find_library(CUDA_RUNTIME_LIBS_STATIC cudart_static
    #   HINT ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
    set(CUDA_RUNTIME_LIBS "${CUDA_RUNTIME_LIBS_DYNAMIC}" CACHE STRING "Path to a library" FORCE)
    # set(CUDA_RUNTIME_LIBS "${CUDA_RUNTIME_LIBS_STATIC}" CACHE STRING "Path to a library" FORCE)
    message(STATUS "CUDA runtime library: ${CUDA_RUNTIME_LIBS}")

    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_EXTENSIONS OFF)

    # string(APPEND CMAKE_CUDA_FLAGS " -g -O2")
    foreach(ARCH ${SyLVER_NVCC_ARCHS})
      string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_${ARCH},code=sm_${ARCH}")
    endforeach()

    ########################################
    # Find cuBLAS library
    find_library(CUBLAS cublas)
    message(STATUS "cuBLAS: ${CUBLAS}")

    ########################################
    # Find cuSOLVER library
    find_library(CUSOLVER cusolver)
    message(STATUS "cuSOLVER: ${CUSOLVER}")

    set(LIBS ${LIBS} ${CUDA_RUNTIME_LIBS})
    set(LIBS ${LIBS} ${CUBLAS})
    set(LIBS ${LIBS} ${CUSOLVER})

  else()
    find_package(CUDA REQUIRED)

    string(APPEND CUDA_NVCC_FLAGS " -std=c++11")
    string(APPEND CUDA_NVCC_FLAGS " -g -O2")

    # string(APPEND CUDA_NVCC_FLAGS "-g")
    # string(APPEND CUDA_NVCC_FLAGS "-g -O2 -arch=compute_60")
    # string(APPEND CUDA_NVCC_FLAGS "-g -O2 -arch=compute_70")

    # set(SyLVER_NVCC_ARCHS "50;60;61;70;75" CACHE STRING "The SM architectures to build code for.")

    # Set NVCC arguments
    foreach(ARCH ${SyLVER_NVCC_ARCHS})
      string(APPEND CUDA_NVCC_FLAGS " -gencode arch=compute_${ARCH},code=sm_${ARCH}")
      # string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_${ARCH},code=sm_${ARCH}")
    endforeach()
    
    MESSAGE( STATUS "CUDA found:         " ${CUDA_FOUND} )
    MESSAGE( STATUS "CUDA host compiler: " ${CUDA_HOST_COMPILER} )
    MESSAGE( STATUS "CUDA nvcc compiler: " ${CUDA_NVCC_EXECUTABLE})
    MESSAGE( STATUS "NVCC FLAGS:         " ${CUDA_NVCC_FLAGS} )

    set(LIBS ${LIBS} ${CUDA_LIBRARIES})
    set(LIBS ${LIBS} ${CUDA_CUBLAS_LIBRARIES})
    set(LIBS ${LIBS} ${CUDA_cusolver_LIBRARY})

    set(LIBS ${LIBS} sylver_cuda)
  endif()

  # find_library(NVIDIA_"/usr/lib64/nvidia")

  # enable_language(CUDA)
  # include(FindCUDA)

  # Allow for dynamic parallelism with CUBLAS
  # set(CUDA_SEPARABLE_COMPILATION TRUE)

  # MAGMA library
  # set(MAGMA_DIR $ENV{MAGMADIR} CACHE PATH "Location of MAGMA Library")
  # set(MAGMA_SRCDIR $ENV{MAGMASRCDIR} CACHE PATH "Location of MAGMA Sources")
  # include_directories(${MAGMA_DIR}/include)
  # include_directories(${MAGMA_SRCDIR}/control)     
  # find_library(MAGMA_LIBRARY magma PATHS ${MAGMA_DIR}/lib)

endif()

########################################
# SPRAL

# Determine SPRAL source dir and build dir

if(SYLVER_SPRAL_USE_INTERNAL)
  # Checkout SPRAL git submodule and build SPRAL
  
  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --force --init --recursive --remote
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()

  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/spral/autogen.sh")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
  endif()

  # SPRAL source directory
  set(SPRAL_SRC_DIR "${PROJECT_SOURCE_DIR}/spral")
  # SPRAL build directory
  set(SPRAL_DIR "${CMAKE_BINARY_DIR}/spral/build")

  # SPRAL configuration options
  set(SPRAL_CONFIGURE_OPTIONS "")
  # Compilers
  set(SPRAL_CONFIGURE_OPTIONS "${SPRAL_CONFIGURE_OPTIONS} CXX=${CMAKE_CXX_COMPILER} FC=${CMAKE_Fortran_COMPILER} CC=${CMAKE_C_COMPILER}")
  # string(APPEND SPRAL_CONFIGURE_OPTIONS " CXX=${CMAKE_CXX_COMPILER} FC=${CMAKE_Fortran_COMPILER} CC=${CMAKE_C_COMPILER}")
  # Compiler flags
  set(SPRAL_CONFIGURE_OPTIONS "${SPRAL_CONFIGURE_OPTIONS} CFLAGS=\"${CMAKE_C_FLAGS_ALL}\" CXXFLAGS=\"${CMAKE_CXX_FLAGS_ALL}\" FCFLAGS=\"${CMAKE_Fortran_FLAGS_ALL}\"")
  # string(APPEND SPRAL_CONFIGURE_OPTIONS " CFLAGS=${CMAKE_C_FLAGS_ALL} CXXFLAGS=${CMAKE_CXX_FLAGS_ALL} FCFLAGS=${CMAKE_Fortran_FLAGS_ALL}")
  # HWLOC
  string(APPEND SPRAL_CONFIGURE_OPTIONS " --with-hwloc=\"${HWLOC_LIBRARIES}\"")
  # Metis
  string(APPEND SPRAL_CONFIGURE_OPTIONS " --with-metis=\"${METIS_LIBRARIES}\"")
  # OpenMP
  if (NOT SYLVER_ENABLE_OMP)
    string(APPEND SPRAL_CONFIGURE_OPTIONS " --disable-openmp")
  endif()
  # CUDA
  if (SYLVER_ENABLE_CUDA)
    # nvcc compiler flags
    if(SYLVER_NATIVE_CUDA)
      string(APPEND SPRAL_CONFIGURE_OPTIONS " NVCCFLAGS=\"${CMAKE_CUDA_FLAGS}\"")
    else()
      string(APPEND SPRAL_CONFIGURE_OPTIONS " NVCC=${CUDA_NVCC_EXECUTABLE}")
      string(APPEND SPRAL_CONFIGURE_OPTIONS " NVCCFLAGS=\"${CUDA_NVCC_FLAGS}\"")
    endif()
  else()
    string(APPEND SPRAL_CONFIGURE_OPTIONS " --disable-gpu")
  endif()

  # Transfor lists into space sperated strings
  string(REPLACE ";" " " BLAS_LIBRARIES_STR "${BLAS_LIBRARIES}")
  string(REPLACE ";" " " LAPACK_LIBRARIES_STR "${LAPACK_LIBRARIES}")
  
  set(SPRAL_CONFIGURE_OPTIONS ${SPRAL_CONFIGURE_OPTIONS} " --with-blas=\"${BLAS_LIBRARIES_STR}\"")
  set(SPRAL_CONFIGURE_OPTIONS ${SPRAL_CONFIGURE_OPTIONS} " --with-lapack=\"${BLAS_LIBRARIES_STR}\"")

  # Create SPRAL config shell script
  file(
    WRITE
    ${CMAKE_BINARY_DIR}/spral/spral-config.sh
    ${SPRAL_SRC_DIR}/configure ${SPRAL_CONFIGURE_OPTIONS}
    )

  # Give execution permission to SPRAL config script
  file(
    COPY ${CMAKE_BINARY_DIR}/spral/spral-config.sh
    DESTINATION ${CMAKE_BINARY_DIR}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

  # TODO: remove config script copy
    
  include(ExternalProject)
  ExternalProject_Add(spral
    SOURCE_DIR ${SPRAL_SRC_DIR}
    PREFIX ${CMAKE_BINARY_DIR}/spral
    PATCH_COMMAND ${SPRAL_SRC_DIR}/autogen.sh
    # Call config script via sh or bash?
    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/spral-config.sh
    BUILD_COMMAND make
    INSTALL_COMMAND ""
    # INSTALL_COMMAND cp ${CMAKE_BINARY_DIR}/spral/build/libspral.a ${CMAKE_BINARY_DIR}/
    BINARY_DIR ${SPRAL_DIR}
    )  

else()
  # Use external SPRAL library
  
  set(SPRAL_DIR "" CACHE PATH "Location of SPRAL Library")
  # Looking for SPRAL library
  if(NOT SPRAL_DIR)
    if (DEFINED ENV{SPRALDIR})
      set(SPRAL_DIR $ENV{SPRALDIR})
    elseif(DEFINED ENV{SPRAL_DIR})
      set(SPRAL_DIR $ENV{SPRAL_DIR})
    else()
      message(STATUS "SPRAL library directory not set and is needed for the compilation of SyLVER. You can provide the path of the SPRAL library directory using either the -DSPRAL_DIR option or the SPRAL_DIR environment variable")
    endif()
  endif()

  # SPRAL source directory
  set(SPRAL_SRC_DIR "" CACHE PATH "Location of SPRAL sources")
  if(NOT SPRAL_SRC_DIR)
    if (DEFINED ENV{SPRALSRCDIR})
      set(SPRAL_SRC_DIR $ENV{SPRALSRCDIR})
    elseif(DEFINED  ENV{SPRAL_SRC_DIR})
      set(SPRAL_SRC_DIR $ENV{SPRAL_SRC_DIR})    
    else()
      message(STATUS "SPRAL source directory not set and is needed for the compilation of SyLVER. You can provide the path of the SPRAL source directory using either the -DSPRAL_SRC_DIR option or the SPRAL_SRC_DIR environment variable")
    endif()
  endif()
  
endif()

# Make sure we have access to SPRAL source directory 
if(SPRAL_SRC_DIR)
  message(STATUS "SPRAL source directory set to: ${SPRAL_SRC_DIR}")
  include_directories(${SPRAL_SRC_DIR})
  include_directories(${SPRAL_SRC_DIR}/src)
else()
  message(FATAL_ERROR "SPRAL source directory not found")
endif()
# Make sure we have access to SPRAL build directory with .mod files
# and libspral.a
if(SPRAL_DIR)
  message(STATUS "SPRAL library directory set to: ${SPRAL_DIR}")
  include_directories(${SPRAL_DIR})
else()
  message(STATUS "SPRAL library directory not found")
endif()

# Locate spral library file
if(SYLVER_SPRAL_USE_INTERNAL)
  set(SPRAL_LIBRARIES ${SPRAL_DIR}/libspral.a)
  set(LIBS ${SPRAL_LIBRARIES} ${LIBS})
else()
  find_library(SPRAL_LIBRARIES spral PATHS ${SPRAL_DIR} ${SPRAL_DIR})
  if(SPRAL_LIBRARIES)
    set(LIBS ${SPRAL_LIBRARIES} ${LIBS})
  elseif((SYLVER_BUILD_EXAMPLES) OR (SYLVER_BUILD_TESTS) OR (SYLVER_BUILD_DRIVERS))
    message(FATAL_ERROR "SPRAL library not found")
  else()
    message(WARNING "SPRAL library not found")
  endif()
endif()

# Include source directory
include_directories(${SyLVER_SOURCE_DIR}/src)

########################################
# Runtime system

if (${SYLVER_RUNTIME} MATCHES "STF")

  add_definitions(-DSPLDLT_USE_STF)

elseif(${SYLVER_RUNTIME} MATCHES "StarPU")

  ########################################
  # StarPU
  add_definitions(-DSPLDLT_USE_STARPU)
  add_definitions(-DSYLVER_HAVE_STARPU)

  find_package(STARPU)
  if (STARPU_FOUND)
    include_directories( ${STARPU_INCLUDE_DIRS} )
    link_directories( ${STARPU_LIBRARY_DIRS} )
    link_libraries( ${STARPU_LIBRARIES} )
    set(LIBS ${LIBS} ${STARPU_LIBRARIES})
    find_path(STARPU_F_MOD_FOUND fstarpu_mod.f90
      HINTS ${STARPU_INCLUDE_DIRS}
      )
    set(STARPU_F_MOD "${STARPU_F_MOD_FOUND}/fstarpu_mod.f90")
    include_directories(${STARPU_INCLUDE_DIRS}) 
  else()
    message(FATAL_ERROR "StarPU not found!")
  endif(STARPU_FOUND)

endif()

if(SYLVER_BUILD_TESTS)
  enable_testing()
  include(CTest)
endif()

add_subdirectory(src)

########################################
# CUTLASS
if (${HAVE_CUTLASS} MATCHES "ON")
  include_directories( ${CUTLASS_HEADERS} )
endif()

################################################################################
# Examples
if(SYLVER_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

################################################################################
# Unit tests
if(SYLVER_BUILD_TESTS)
  add_subdirectory(tests)
endif()

################################################################################
# Test drivers
if(SYLVER_BUILD_DRIVERS)
  add_subdirectory(drivers)
endif()

sylver_install()

################################################################################
# Print summary

message("--------------------------------------------------------------------------------")
message("Configuration of SyLVER done.")
message(" ")
message("Options:")
message(" ")
# Print options summuary 
message( " Runtime: " ${SYLVER_RUNTIME})
message( " Enable CUDA: " ${SYLVER_ENABLE_CUDA})
message( " Enable OpenMP: " ${SYLVER_ENABLE_OMP})
message( " Use internal SPRAL: " ${SYLVER_SPRAL_USE_INTERNAL})
message( " Build examples: " ${SYLVER_BUILD_EXAMPLES})
message( " Build tests: " ${SYLVER_BUILD_TESTS})
message( " Build drivers: " ${SYLVER_BUILD_DRIVERS})
message( " Profiling: " ${SYLVER_ENABLE_PROFILING})
message( " CUTLASS: " ${SYLVER_ENABLE_CUTLASS})
message(" ")
message(
  "Build type: ${CMAKE_BUILD_TYPE}\n"
  "Fortran compiler: ${CMAKE_Fortran_COMPILER} (${CMAKE_Fortran_COMPILER_ID})\n"
  "Fortran compiler flags: ${CMAKE_Fortran_FLAGS_ALL}\n"
  "C compiler:  ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID})\n"
  "C compiler flags: ${CMAKE_Fortran_FLAGS_ALL}\n"
  "CXX compiler:  ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})\n"
  "CXX compiler flags: ${CMAKE_CXX_FLAGS_ALL}\n"
  "Linker Flags: ${CMAKE_EXE_LINKER_FLAGS}\n"
  # "Linker lang: ${LINKER_LANGUAGE}"
  )
message(" ")
message("BLAS libraries: ${BLAS_LIBRARIES}")
message("LAPACK libraries: ${LAPACK_LIBRARIES}")
message(" ")
if(${SYLVER_RUNTIME} MATCHES "StarPU")
  message("Runtime system: StarPU")
  message("StarPU libraries: ${STARPU_LIBRARIES}")
  message(" ")
endif()
if(SYLVER_ENABLE_CUDA)
  if(SYLVER_NATIVE_CUDA)
    message(
      "CUDA host compiler: ${CMAKE_CUDA_HOST_COMPILER}\n"
      "CUDA libraries: ${CUDA_RUNTIME_LIBS}\n"
      "CUBLAS libraries: ${CUBLAS}\n"
      "NVVC compiler: ${CMAKE_CUDA_COMPILER}\n"
      "NVCC FLAGS: ${CMAKE_CUDA_FLAGS}\n"
      )
  else()
    message(
      "CUDA host compiler: ${CUDA_HOST_COMPILER}\n"
      "CUDA libraries: ${CUDA_LIBRARIES}\n"
      "CUBLAS libraries: ${CUDA_CUBLAS_LIBRARIES}\n"
      "NVVC compiler: ${CUDA_NVCC_EXECUTABLE}\n"
      "NVCC FLAGS: ${CUDA_NVCC_FLAGS}\n"
      )
  endif()
  message(" ")
endif()
# if (${USE_OMP} MATCHES ON)
#   message(
#     "OpenMP CXX libraries: ${OpenMP_CXX_LIBRARIES}\n"
#     "OpenMP Fortran libraries: ${OpenMP_Fortran_LIBRARIES}\n"
#     )
# endif()
message("--------------------------------------------------------------------------------")
