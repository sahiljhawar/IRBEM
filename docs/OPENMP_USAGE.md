# OpenMP Thread-Safe Example for IRBEM

This example demonstrates how to use the IRBEM library in a thread-safe manner with OpenMP.

**Related Issue**: See [PRBEM/IRBEM#65](https://github.com/PRBEM/IRBEM/issues/65) for background on parallelization requirements.

## Overview

The IRBEM library has been partially updated to support OpenMP thread-level parallelism by adding `THREADPRIVATE` directives to COMMON blocks. This allows multiple threads to safely execute library functions in parallel.

## Building with OpenMP Support

### Linux

```bash
# Clean previous build
make OS=linux64 ENV=gfortran64 clean

# Build with OpenMP support
make OS=linux64 ENV=gfortran64 FFLAGS="-fpic -fopenmp -fno-second-underscore -std=legacy -ffixed-line-length-none" LDFLAGS="-shared -fopenmp" all

# Install
make OS=linux64 ENV=gfortran64 install
```

### Mac OSX

```bash
# Clean previous build
make OS=osx64 ENV=gfortran64 clean

# Build with OpenMP support
make OS=osx64 ENV=gfortran64 FFLAGS="-fpic -fopenmp -fno-second-underscore -std=legacy -ffixed-line-length-none" LDFLAGS="-shared -fopenmp" all

# Install
make OS=osx64 ENV=gfortran64 install
```

## Status of Thread-Safe Implementation

### Completed
- ✅ `onera_desp_lib.f` - Main wrapper functions with THREADPRIVATE directives
- ✅ Build system updated to support OpenMP
- ✅ Documentation created

### TODO
The following files still need THREADPRIVATE directives added to all COMMON blocks:
- `init_nouveau.f` (32 COMMON blocks)
- `Tsy_and_Sit07_Jul2017.f` (30 COMMON blocks)
- `Tsy_and_Sit07_2015.f` (30 COMMON blocks)
- `t01_s.f`, `Tsyganenko04.f`, `Tsyganenko01.f` (17 each)
- `geopack_08.f`, `Tsyganenko96.f` (15 each)
- `heliospheric_transformation.f` (11 blocks)
- And 30+ additional files

## C Example with OpenMP

```c
#include <stdio.h>
#include <omp.h>

// IRBEM function declaration (adjust based on actual header)
extern void make_lstar1_(int *ntime, int *kext, int *options, 
                         int *sysaxes, int *iyear, int *idoy,
                         double *UT, double *xIN1, double *xIN2, 
                         double *xIN3, double *maginput,
                         double *Lm, double *Lstar, double *BLOCAL,
                         double *BMIN, double *XJ, double *MLT);

int main() {
    const int ntimes = 1000;
    const int nthreads = 4;
    
    // Allocate arrays for inputs and outputs
    int *iyear = malloc(ntimes * sizeof(int));
    int *idoy = malloc(ntimes * sizeof(int));
    double *UT = malloc(ntimes * sizeof(double));
    // ... allocate other arrays ...
    
    // Initialize input data
    for (int i = 0; i < ntimes; i++) {
        iyear[i] = 2020;
        idoy[i] = 100 + i % 265;
        UT[i] = (double)i / ntimes * 24.0;
        // ... initialize other parameters ...
    }
    
    // Set number of OpenMP threads
    omp_set_num_threads(nthreads);
    
    // Parallel computation
    printf("Computing %d L* values using %d threads...\\n", ntimes, nthreads);
    
    #pragma omp parallel for
    for (int i = 0; i < ntimes; i++) {
        int ntime = 1;  // Process one time point per thread
        int kext = 5;   // External field model
        int options[5] = {0, 0, 0, 0, 0};
        int sysaxes = 1;  // GDZ coordinates
        
        // Each thread calls the Fortran library independently
        make_lstar1_(&ntime, &kext, options, &sysaxes,
                    &iyear[i], &idoy[i], &UT[i],
                    &xIN1[i], &xIN2[i], &xIN3[i],
                    &maginput[i*25],  // 25 magnetic input parameters
                    &Lm[i], &Lstar[i], &BLOCAL[i],
                    &BMIN[i], &XJ[i], &MLT[i]);
        
        if (i % 100 == 0) {
            #pragma omp critical
            printf("Thread %d: Processed point %d, L* = %f\\n",
                   omp_get_thread_num(), i, Lstar[i]);
        }
    }
    
    printf("Computation complete!\\n");
    
    // Free memory
    free(iyear);
    free(idoy);
    free(UT);
    // ... free other arrays ...
    
    return 0;
}
```

### Compiling the Example

```bash
gcc -fopenmp -o example_openmp example_openmp.c -L. -lirbem -lgfortran -lm
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./example_openmp
```

## Python Example with OpenMP

```python
import numpy as np
from multiprocessing.pool import ThreadPool
import ctypes
import os

# Load IRBEM library
lib = ctypes.CDLL('./libirbem.so')

# Configure make_lstar1 function signature
lib.make_lstar1_.argtypes = [
    ctypes.POINTER(ctypes.c_int),  # ntime
    ctypes.POINTER(ctypes.c_int),  # kext
    ctypes.POINTER(ctypes.c_int * 5),  # options
    ctypes.POINTER(ctypes.c_int),  # sysaxes
    # ... add remaining parameters ...
]

def compute_lstar(params):
    """Compute L* for a single time point."""
    i, year, doy, ut, x1, x2, x3 = params
    
    # Initialize inputs
    ntime = ctypes.c_int(1)
    kext = ctypes.c_int(5)
    options = (ctypes.c_int * 5)(0, 0, 0, 0, 0)
    sysaxes = ctypes.c_int(1)
    
    # ... create other parameters ...
    
    # Call Fortran function
    lib.make_lstar1_(
        ctypes.byref(ntime),
        ctypes.byref(kext),
        ctypes.byref(options),
        ctypes.byref(sysaxes),
        # ... pass remaining parameters ...
    )
    
    return lstar.value

# Create parameter list
params_list = [(i, 2020, 100+i, i/100.0, 6.6, 0.0, 0.0) 
               for i in range(1000)]

# Parallel computation using thread pool
with ThreadPool(4) as pool:
    results = pool.map(compute_lstar, params_list)

print(f"Computed {len(results)} L* values")
```

## Performance Notes

1. **Thread Overhead**: Each thread needs its own copy of COMMON block data. With many threads, memory usage increases.

2. **Initialization**: The first call in each thread initializes thread-private data. Expect some overhead.

3. **Scaling**: Best performance with thread count ≤ number of physical CPU cores.

4. **MPI Alternative**: For distributed computing, the existing MPI example in `example/multi_Lstar_hmin.c` remains the recommended approach.

## Hybrid MPI + OpenMP

For maximum performance on clusters:

```c
#include <mpi.h>
#include <omp.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Each MPI process uses multiple OpenMP threads
    omp_set_num_threads(4);
    
    // Distribute work across processes
    int work_per_process = total_work / size;
    int my_start = rank * work_per_process;
    int my_end = (rank + 1) * work_per_process;
    
    // Each process parallelizes its chunk with OpenMP
    #pragma omp parallel for
    for (int i = my_start; i < my_end; i++) {
        // Compute...
    }
    
    // Gather results
    MPI_Gather(/*...*/);
    
    MPI_Finalize();
    return 0;
}
```

Compile with:
```bash
mpicc -fopenmp -o hybrid hybrid.c -lirbem -lgfortran -lm
mpirun -np 4 ./hybrid
```

## Troubleshooting

### "Segmentation fault" or race conditions

If you encounter crashes or incorrect results:

1. Ensure library was compiled with `-fopenmp`
2. Verify all COMMON blocks in used functions have THREADPRIVATE directives
3. Check that you're not sharing data between threads unsafely

### Link errors

If you get undefined references to OpenMP symbols:
```bash
# Add -fopenmp to both compile and link flags
gcc -fopenmp -c file.c
gcc -fopenmp -o program file.o -lirbem -lgfortran -lm
```

### Poor scaling

If performance doesn't improve with more threads:

1. Reduce thread count to match physical cores
2. Consider MPI for better scaling across nodes
3. Profile to identify bottlenecks
4. Use hybrid MPI+OpenMP for large clusters

## Completing the Implementation

To make all functions thread-safe, THREADPRIVATE directives must be added to all remaining files. The pattern is:

```fortran
      COMMON /blockname/var1,var2,var3
!$OMP THREADPRIVATE(/blockname/)
```

Add this immediately after each COMMON block declaration in every subroutine/function.

## Further Reading

- OpenMP Fortran Specification: https://www.openmp.org/
- IRBEM Documentation: `docs/PARALLELIZATION_GUIDE.md`
- MPI Example: `example/multi_Lstar_hmin.c`
