!***************************************************************************************************
! Copyright 2024 IRBEM Contributors
!
! This file is part of IRBEM-LIB.
!
!    IRBEM-LIB is free software: you can redistribute it and/or modify
!    it under the terms of the GNU Lesser General Public License as published by
!    the Free Software Foundation, either version 3 of the License, or
!    (at your option) any later version.
!
!    IRBEM-LIB is distributed in the hope that it will be useful,
!    but WITHOUT ANY WARRANTY; without even the implied warranty of
!    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!    GNU Lesser General Public License for more details.
!
!    You should have received a copy of the GNU Lesser General Public License
!    along with IRBEM-LIB.  If not, see <http://www.gnu.org/licenses/>.
!
!***************************************************************************************************
! OpenMP THREADPRIVATE directives for COMMON blocks
!
! This file contains OpenMP THREADPRIVATE directives for all COMMON blocks
! used in the IRBEM library. Including this file after COMMON block declarations
! makes those blocks thread-private, allowing safe parallel execution with OpenMP.
!
! To use:
!   1. Include this file after all COMMON block declarations in each subroutine
!   2. Compile with OpenMP enabled: -fopenmp flag for gfortran
!   3. Use OpenMP pragmas in calling code to parallelize loops
!
! Note: Each thread will have its own copy of all COMMON block variables.
! This increases memory usage but enables thread-safe parallel execution.
!
!***************************************************************************************************

! Core magnetic field and coordinate transformation blocks
!$OMP THREADPRIVATE(/magmod/)
!$OMP THREADPRIVATE(/flag_L/)
!$OMP THREADPRIVATE(/dipigrf/)
!$OMP THREADPRIVATE(/dip_ang/)

! Solar and temporal blocks
!$OMP THREADPRIVATE(/Soleil/)
!$OMP THREADPRIVATE(/sundip/)
!$OMP THREADPRIVATE(/sunMAH/)
!$OMP THREADPRIVATE(/a2000_time/)

! Driver and index blocks
!$OMP THREADPRIVATE(/drivers/)
!$OMP THREADPRIVATE(/index/)

! Coordinate and geometry blocks
!$OMP THREADPRIVATE(/GEOPACK1/)
!$OMP THREADPRIVATE(/GEOPACK2/)
!$OMP THREADPRIVATE(/rconst/)
!$OMP THREADPRIVATE(/calotte/)
!$OMP THREADPRIVATE(/calotte2/)
!$OMP THREADPRIVATE(/COORD11/)
!$OMP THREADPRIVATE(/COORD21/)

! Magnetic field model specific blocks
!$OMP THREADPRIVATE(/TAIL/)
!$OMP THREADPRIVATE(/BIRKPAR/)
!$OMP THREADPRIVATE(/DPHI_B_RHO0/)
!$OMP THREADPRIVATE(/DTHETA/)
!$OMP THREADPRIVATE(/MODENUM/)
!$OMP THREADPRIVATE(/WARP/)
!$OMP THREADPRIVATE(/LOOPDIP1/)
!$OMP THREADPRIVATE(/RCPAR/)
!$OMP THREADPRIVATE(/COR1/)
!$OMP THREADPRIVATE(/COR2/)
!$OMP THREADPRIVATE(/DX1/)
!$OMP THREADPRIVATE(/RHDR/)

! Model coefficient and parameter blocks
!$OMP THREADPRIVATE(/dgrf/)
!$OMP THREADPRIVATE(/G/)
!$OMP THREADPRIVATE(/MODEL/)
!$OMP THREADPRIVATE(/intfield/)
!$OMP THREADPRIVATE(/TS07D_DATA/)
!$OMP THREADPRIVATE(/TSE/)
!$OMP THREADPRIVATE(/TSO/)
!$OMP THREADPRIVATE(/TSS/)

! Radiation model blocks
!$OMP THREADPRIVATE(/RH0/)
!$OMP THREADPRIVATE(/RH0_t01_s/)
!$OMP THREADPRIVATE(/RH0_tsyg04/)
!$OMP THREADPRIVATE(/ELEMAX/)
!$OMP THREADPRIVATE(/ELEMIN/)
!$OMP THREADPRIVATE(/PROMAX/)
!$OMP THREADPRIVATE(/PROMIN/)
!$OMP THREADPRIVATE(/sd2elbr/)
!$OMP THREADPRIVATE(/sd2elbrTab/)
!$OMP THREADPRIVATE(/sd2pro/)
!$OMP THREADPRIVATE(/sd2proTab/)

! Debug blocks
!$OMP THREADPRIVATE(/DebugHelp/)
