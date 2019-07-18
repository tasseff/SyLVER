# SyLVER

[![Documentation Status](https://readthedocs.org/projects/sylver/badge/?version=latest)](https://sylver.readthedocs.io/en/latest/?badge=latest)

SyLVER is a sparse direct solver for symmetric systems which may be
either positive-definite or indefinite. It implements DAG-based
algorithms that enable an efficient exploitation of multicore
architectures and heterogeneous GPU-accelerated systems. The code has
been developed in the context of the EU H2020
[NLAFET](http://www.nlafet.eu/) project. The parallel implementation
relies on the [StarPU](http://starpu.gforge.inria.fr/) runtime system
developed and maintained by the STORM team at Inria Bordeaux
Sud-Ouest.

# Installation 

The compilation of the code is handled by the
[CMake](https://cmake.org/) tools. For example, the compilation can be
achieved as follow:

```bash
mkdir build # create build directory
cd build 
cmake <path-to-source> -DRUNTIME=StarPU # configure compilation
make # run compilation 
```

## Third-party libraries ##

### SPRAL ###

[SPRAL](https://github.com/ralna/spral) is an open-source library for
sparse linear algebra and associated algorithm and has several
important features used in SyLVER. The latest release of SPRAL can be
found on its [GitHub
repository](https://github.com/ralna/spral/releases). The compilation
of SPRAL is handled by autotools and for example can be done as follow
when using the GCC compilers:

```bash
cd spral
mkdir build
cd build
../configure CXX=g++ FC=gfortran CC=gcc CFLAGS="-g -O2 -march=native" CXXFLAGS="-g -O2 -march=native" FCFLAGS="-g -O2 -march=native" --with-metis="-L/path/to/metis -lmetis" --with-blas="-L/path/to/blas -lblas" --with-lapack="-L/path/to/lapack -llapack" --disable-openmp --disable-gpu
make
```
Note that the compilation flags used for SPRAL **must match** the flags
used in the compilation of SyLVER. Here we use the flags `-g -O2
-march=native` that correspond to the `RelWithDebInfo` build type in
SyLVER.

Here we use the `--disable-openmp` option because SyLVER works with
the serial version of SPRAL. Additionally, in this example we disabled
the compilation of the SPRAL GPU kernels using the `--disable-gpu`
option.

**Sequential version** of BLAS and LAPACK should be used. We recommend
using the [MKL](https://software.intel.com/mkl) library for best
performance on Intel machines and
[ESSL](https://www.ibm.com/support/knowledgecenter/en/SSFHY8/essl_welcome.html)
on IBM machines. The [MKL link line
advisor](https://software.intel.com/en-us/articles/intel-mkl-link-line-advisor)
can be useful to fill the `--with-blas` and `--with-lapack` options.

When compiling SyLVER you need to provide both the path to the SPRAL
source directory which can be given using the `-DSPRAL_SRC_DIR` CMake
option or the `SPRAL_SRC_DIR` environment variable and the path to the
SPRAL library which can be given using the `-DSPRAL_DIR` CMake option
or the `SPRAL_DIR` environment variable.

### MeTiS ###

The [MeTiS](http://glaros.dtc.umn.edu/gkhome/metis/metis/overview)
partitioning library is needed by the SPRAL library and therefore,
needed when linking the SyLVER package for building examples and test
drivers.

When compiling SyLVER you can provide the path to the MeTiS library
using either `-DMETIS_DIR` CMake option or the `METIS_DIR`
environment variable.

### hwloc ###

The [hwloc](https://www.open-mpi.org/projects/hwloc/) library is
topology discovery library which is necessary for linking the examples
and test drivers if SPRAL was compiled with it. In this case, the
library path can be given to CMake using either the `-DHWLOC_DIR` or
the `HWLOC_DIR` environment variable.

### Runtime system ###

By default, CMake will configure the compilation for a serial version
of SyLVER that can be explicitly requested using the option
`-DRUNTIME=STF`.

The `-DRUNTIME=StarPU` enables the compilation of the parallel version
of SyLVER using [StarPU runtime
system](http://starpu.gforge.inria.fr/). In this case the StarPU
version needs to be at least 1.3.0. The StarPU library is found with
the `FindSTARPU.cmake` script located in the `cmake/Modules`
directory. Note that, for this script to be able to find the StarPU
library, you need to set the environment variable `STARPU_DIR` to the
path of you StarPU install base directory.

### BLAS and LAPACK ###

The BLAS and LAPACK libraries play an important role in the
performance of the solver. We recommend using the
[MKL](https://software.intel.com/mkl) library for best performance on
Intel machines and the
[ESSL](https://www.ibm.com/support/knowledgecenter/en/SSFHY8/essl_welcome.html)
library when running on IBM machines. Alternative BLAS and LAPACK
libraries include [OpenBLAS](https://www.openblas.net/). Note that
SyLVER should be linked against the **sequential** BLAS and LAPACK
libraries.

These libraries are found via the CMake scripts
[FindBLAS](https://cmake.org/cmake/help/latest/module/FindBLAS.html)
and
[FindLAPACK](https://cmake.org/cmake/help/latest/module/FindBLAS.html)
and therefore it is possible to use the options `-DBLA_VENDOR` to
indicate which libraries to use. For example:

```bash
cmake <path-to-source> -DBLA_VENDOR=Intel10_64lp_seq # configure compilation
```

selects and locates the sequential BLAS and LAPACK implementation for
the compilation and when linking test drivers, example and tests.

If CMake is unable to locate the requested libraries via the
`-DBLA_VENDOR`, it is still possible to give them explicitly using the
`-DLLBAS` and `-DLLAPACK` options. For example:

```bash
cmake <path-to-source> -DLBLAS="-L/path/to/blas -lblas" -DLLAPACK="-L/path/to/lapack -llapack" # configure compilation
```
