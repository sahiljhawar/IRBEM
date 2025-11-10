/**
 * Thread-Safe C++ Wrapper for IRBEM Library
 * 
 * This wrapper provides thread-safe access to the IRBEM Fortran library
 * by using a mutex to serialize all library calls. While this doesn't
 * enable true parallelism within the library, it prevents race conditions
 * and allows safe use from multi-threaded applications.
 * 
 * For true parallelism, use MPI (see example/multi_Lstar_hmin.c).
 * 
 * Copyright 2024 IRBEM Contributors
 * Licensed under LGPL v3+
 */

#ifndef IRBEM_THREADSAFE_H
#define IRBEM_THREADSAFE_H

#include <mutex>
#include <stdexcept>

// Fortran function declarations (adjust based on actual library interface)
extern "C" {
    void make_lstar1_(int* ntime, int* kext, int* options, int* sysaxes,
                      int* iyearsat, int* idoy, double* UT,
                      double* xIN1, double* xIN2, double* xIN3,
                      double* maginput, double* Lm, double* Lstar,
                      double* BLOCAL, double* BMIN, double* XJ, double* MLT);
    
    void drift_bounce_orbit2_1_(int* kext, int* options, int* sysaxes,
                                int* iyear, int* idoy, double* UT,
                                double* x1, double* x2, double* x3,
                                double* alpha, double* maginput, double* R0,
                                double* Lm, double* Lstar, double* Blocal,
                                double* Bmin, double* Bmir, double* J,
                                double* posit, int* ind, double* hmin,
                                double* hmin_lon);
}

namespace IRBEM {

/**
 * Thread-safe wrapper for IRBEM library functions.
 * 
 * This class uses a static mutex to ensure only one thread at a time
 * can execute IRBEM library functions.  This prevents race conditions
 * from concurrent access to Fortran COMMON blocks.
 * 
 * Usage:
 *   ThreadSafe irbem;
 *   irbem.make_lstar(...);  // Thread-safe call
 */
class ThreadSafe {
private:
    static std::mutex library_mutex;
    
public:
    /**
     * Thread-safe wrapper for make_lstar1
     * 
     * Computes L* (Roederer L parameter) for given position and time.
     * 
     * @param ntime Number of time points (usually 1 for single call)
     * @param kext External magnetic field model
     * @param options Array of 5 options
     * @param sysaxes Coordinate system
     * @param iyearsat Year
     * @param idoy Day of year
     * @param UT Universal time (hours)
     * @param xIN1,xIN2,xIN3 Position coordinates
     * @param maginput Magnetic field parameters (25 values)
     * @param Lm,Lstar,BLOCAL,BMIN,XJ,MLT Output parameters
     */
    void make_lstar(int ntime, int kext, int* options, int sysaxes,
                    int* iyearsat, int* idoy, double* UT,
                    double* xIN1, double* xIN2, double* xIN3,
                    double* maginput, double* Lm, double* Lstar,
                    double* BLOCAL, double* BMIN, double* XJ, double* MLT) {
        std::lock_guard<std::mutex> lock(library_mutex);
        make_lstar1_(&ntime, &kext, options, &sysaxes, iyearsat, idoy, UT,
                     xIN1, xIN2, xIN3, maginput, Lm, Lstar,
                     BLOCAL, BMIN, XJ, MLT);
    }
    
    /**
     * Thread-safe wrapper for drift_bounce_orbit2_1
     * 
     * Computes drift and bounce orbits for trapped particles.
     */
    void drift_bounce_orbit(int kext, int* options, int sysaxes,
                           int iyear, int idoy, double UT,
                           double x1, double x2, double x3, double alpha,
                           double* maginput, double R0,
                           double* Lm, double* Lstar, double* Blocal,
                           double* Bmin, double* Bmir, double* J,
                           double* posit, int* ind, double* hmin,
                           double* hmin_lon) {
        std::lock_guard<std::mutex> lock(library_mutex);
        drift_bounce_orbit2_1_(&kext, options, &sysaxes, &iyear, &idoy, &UT,
                              &x1, &x2, &x3, &alpha, maginput, &R0,
                              Lm, Lstar, Blocal, Bmin, Bmir, J,
                              posit, ind, hmin, hmin_lon);
    }
    
    // Add more wrapper functions as needed...
};

// Initialize static mutex
std::mutex ThreadSafe::library_mutex;

} // namespace IRBEM

#endif // IRBEM_THREADSAFE_H
