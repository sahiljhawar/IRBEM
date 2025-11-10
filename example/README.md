# IRBEM Examples

This directory contains examples demonstrating different ways to use the IRBEM library, including parallelization approaches.

## Examples

### multi_Lstar_hmin.c - MPI Parallelization (RECOMMENDED)

**Purpose**: Demonstrates process-level parallelism using MPI  
**Status**: ✅ Production-ready  
**Best for**: HPC clusters, large-scale computations

Computes L* and h_min values for multiple time points using MPI to distribute work across processes.

**Compile**:
```bash
mpicc -o multi_Lstar_hmin multi_Lstar_hmin.c -lgfortran -lm -lirbem
```

**Run**:
```bash
# Create input file first (binary format, see source for details)
mpirun -np 4 ./multi_Lstar_hmin input.dat output.dat
```

**Key features**:
- True parallelism across multiple processes
- Each process has isolated memory (no race conditions)
- Scales to 100s-1000s of processes
- Uses master-slave architecture for work distribution

### example_threadsafe.cpp - Thread-Safe C++ Wrapper

**Purpose**: Demonstrates safe multi-threaded usage with mutex protection  
**Status**: ✅ Safe but serialized (no performance gain)  
**Best for**: Desktop applications, thread-safety without parallelism

Shows how to safely call IRBEM from multiple threads using the C++ wrapper with mutex protection.

**Compile**:
```bash
g++ -std=c++11 -pthread -o example_threadsafe example_threadsafe.cpp \
    -L../bin -lirbem.linux64.gfortran64 -lgfortran -lm
```

**Run**:
```bash
export LD_LIBRARY_PATH=../bin:$LD_LIBRARY_PATH
./example_threadsafe
```

**Key features**:
- Thread-safe (prevents crashes in multi-threaded apps)
- Uses mutex to serialize library calls
- No performance benefit, but safe from race conditions
- Good for applications that are already multi-threaded

## Choosing the Right Approach

| Scenario | Recommended Example | Parallel? | Scaling |
|----------|-------------------|-----------|---------|
| HPC cluster computation | `multi_Lstar_hmin.c` (MPI) | ✅ Yes | Excellent |
| Multi-core workstation | `multi_Lstar_hmin.c` (MPI) | ✅ Yes | Good |
| Thread-safe desktop app | `example_threadsafe.cpp` | ❌ No | Serial only |
| Hybrid HPC | MPI + Thread wrapper | ⚠️ Partial | Good |

## Performance Notes

### MPI Approach (`multi_Lstar_hmin.c`)
- **Speedup**: Near-linear with number of processes
- **Memory**: ~50-100 MB per process
- **Optimal**: Number of processes ≤ number of compute nodes
- **Limitations**: Communication overhead between processes

### Thread-Safe Wrapper (`example_threadsafe.cpp`)
- **Speedup**: None (serialized by mutex)
- **Memory**: Single copy of library in memory
- **Optimal**: Use for safety, not performance
- **Limitations**: Only one thread executes library code at a time

## Building the Examples

### Prerequisites
- MPI implementation (OpenMPI, MPICH, etc.) for MPI example
- C++ compiler with C++11 support for thread-safe example
- Compiled IRBEM library (`libirbem.so` or similar)

### Build IRBEM Library First
```bash
cd ..
make OS=linux64 ENV=gfortran64 all
```

### Build MPI Example
```bash
cd example
mpicc -o multi_Lstar_hmin multi_Lstar_hmin.c \
      -L../bin -lirbem.linux64.gfortran64 -lgfortran -lm
```

### Build Thread-Safe Example
```bash
cd example
g++ -std=c++11 -pthread -o example_threadsafe example_threadsafe.cpp \
    -L../bin -lirbem.linux64.gfortran64 -lgfortran -lm
```

## Input Data Format

### For MPI Example (`multi_Lstar_hmin.c`)

The input file must be in binary format with the following structure:

1. Header (100 bytes):
   - Magic string: "multi_Lstar_hmin"
   - Flags byte indicating if maginput varies per time

2. Parameters:
   - ntimes (int32): Number of time points
   - kext (int32): External field model
   - options (5 × int32): Calculation options
   - sysaxes (int32): Coordinate system
   - R0 (double): Reference distance

3. Time series data:
   - maginput (25 × ntimes × double): Magnetic field parameters
   - iyear (ntimes × int32): Years
   - idoy (ntimes × int32): Day of year
   - UT (ntimes × double): Universal time
   - x1, x2, x3 (ntimes × double each): Position coordinates
   - alpha (ntimes × double): Pitch angle

See the source code for details on creating properly formatted input files.

## Further Documentation

- **Parallelization Guide**: `../docs/PARALLELIZATION_GUIDE.md`
- **OpenMP Guide**: `../docs/OPENMP_USAGE.md`
- **Thread-Safe Header**: `../matlab/irbem_threadsafe.h`

## Troubleshooting

### MPI Example

**Problem**: `mpicc: command not found`  
**Solution**: Install MPI: `sudo apt-get install openmpi-bin libopenmpi-dev`

**Problem**: Library not found at runtime  
**Solution**: Set library path: `export LD_LIBRARY_PATH=../bin:$LD_LIBRARY_PATH`

**Problem**: Incorrect results or crashes  
**Solution**: Verify input file format, check that all processes have access to library

### Thread-Safe Example

**Problem**: Undefined reference to pthread  
**Solution**: Add `-pthread` flag to compile command

**Problem**: No speedup with multiple threads  
**Solution**: Expected! This example uses mutex for safety, not parallelism.  
Use MPI example for actual parallel speedup.

**Problem**: Segmentation fault  
**Solution**: Ensure library path is set correctly with `LD_LIBRARY_PATH`

## Contact

For questions about IRBEM parallelization:
- See: `../docs/PARALLELIZATION_GUIDE.md`
- GitHub Issues: https://github.com/PRBEM/IRBEM/issues
