/**
 * Example: Thread-safe usage of IRBEM library
 * 
 * This example demonstrates how to safely use the IRBEM library from
 * multiple threads using the C++ wrapper with mutex protection.
 * 
 * Compile:
 *   g++ -std=c++11 -pthread -o example_threadsafe example_threadsafe.cpp -L../bin -lirbem.linux64.gfortran64 -lgfortran -lm
 * 
 * Run:
 *   LD_LIBRARY_PATH=../bin:$LD_LIBRARY_PATH ./example_threadsafe
 */

#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include "../matlab/irbem_threadsafe.h"

// Constants
constexpr int NTIME_MAX = 100000;
constexpr double BADDATA = -1e31;

void compute_lstar_for_range(int thread_id, int start_idx, int end_idx) {
    IRBEM::ThreadSafe irbem;
    
    // Allocate arrays for this thread's work
    std::vector<int> iyearsat(NTIME_MAX);
    std::vector<int> idoy(NTIME_MAX);
    std::vector<double> UT(NTIME_MAX);
    std::vector<double> xIN1(NTIME_MAX), xIN2(NTIME_MAX), xIN3(NTIME_MAX);
    std::vector<double> maginput(25 * NTIME_MAX);
    std::vector<double> Lm(NTIME_MAX), Lstar(NTIME_MAX);
    std::vector<double> BLOCAL(NTIME_MAX), BMIN(NTIME_MAX);
    std::vector<double> XJ(NTIME_MAX), MLT(NTIME_MAX);
    
    // Configuration
    int kext = 5;  // External field model (T89)
    int options[5] = {0, 0, 0, 0, 0};
    int sysaxes = 1;  // GDZ coordinates
    
    std::cout << "Thread " << thread_id << ": Processing indices " 
              << start_idx << " to " << end_idx << std::endl;
    
    for (int i = start_idx; i < end_idx; i++) {
        // Set up input parameters for this calculation
        iyearsat[0] = 2020;
        idoy[0] = 100 + (i % 265);
        UT[0] = static_cast<double>(i % 24);
        
        // Position: LEO orbit example (altitude ~500 km)
        xIN1[0] = 6371.0 + 500.0;  // radial distance in km
        xIN2[0] = static_cast<double>(i % 180) - 90.0;  // latitude
        xIN3[0] = static_cast<double>((i * 15) % 360);  // longitude
        
        // Magnetic field parameters (simplified - normally would set based on conditions)
        for (int j = 0; j < 25; j++) {
            maginput[j] = 0.0;
        }
        maginput[0] = 2.0;  // Kp index
        
        // Call IRBEM library (thread-safe via mutex)
        int ntime = 1;
        try {
            irbem.make_lstar(ntime, kext, options, sysaxes,
                           iyearsat.data(), idoy.data(), UT.data(),
                           xIN1.data(), xIN2.data(), xIN3.data(),
                           maginput.data(), Lm.data(), Lstar.data(),
                           BLOCAL.data(), BMIN.data(), XJ.data(), MLT.data());
            
            if (Lstar[0] != BADDATA && i % 10 == 0) {
                std::cout << "Thread " << thread_id << ": Index " << i 
                          << ", L* = " << Lstar[0] << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Thread " << thread_id << ": Error at index " << i 
                      << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Thread " << thread_id << ": Complete!" << std::endl;
}

int main(int argc, char* argv[]) {
    const int num_threads = 4;
    const int total_points = 100;
    const int points_per_thread = total_points / num_threads;
    
    std::cout << "IRBEM Thread-Safe Example" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << "Computing " << total_points << " L* values using " 
              << num_threads << " threads" << std::endl;
    std::cout << "NOTE: Library calls are serialized via mutex for thread safety" 
              << std::endl << std::endl;
    
    // Create threads
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++) {
        int start = i * points_per_thread;
        int end = (i == num_threads - 1) ? total_points : (i + 1) * points_per_thread;
        threads.emplace_back(compute_lstar_for_range, i, start, end);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << std::endl << "All computations complete!" << std::endl;
    std::cout << std::endl;
    std::cout << "NOTE: For true parallelism, use MPI (see example/multi_Lstar_hmin.c)" 
              << std::endl;
    
    return 0;
}
