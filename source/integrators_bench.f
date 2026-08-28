C***********************************************************************
C* Alternate field-line stepper (RK3) for benchmarking against the
C* legacy fixed-step RK4 (sksyst) used throughout the L* / drift-shell
C* code.
C*
C* Shares sksyst's calling convention:
C*     CALL xxxxx(h, xx, x2, Bl, Ifail)
C* where xx -> x2 is an arc-length step of signed length h along the
C* field line (dx/ds = B/|B|), and Bl is |B| at the returned point x2.
C*
C* sksyst() below is now a thin dispatcher controlled by INTEG_METHOD
C* (see integ_sel.cmn). Method 0 reproduces the original RK4 exactly.
C***********************************************************************
       BLOCK DATA INTEG_SEL_BLOCK
       IMPLICIT NONE
       INCLUDE 'integ_sel.cmn'
       INCLUDE 'integ_stats.cmn'
       DATA INTEG_METHOD /0/
       DATA INTEG_NCHAMP /0/
       DATA INTEG_NSTEPS /0/
       END
C
       SUBROUTINE SET_INTEG_METHOD(im)
       IMPLICIT NONE
       INTEGER*4 im
       INCLUDE 'integ_sel.cmn'
       INTEG_METHOD = im
       RETURN
       END
C
       SUBROUTINE GET_INTEG_METHOD(im)
       IMPLICIT NONE
       INTEGER*4 im
       INCLUDE 'integ_sel.cmn'
       im = INTEG_METHOD
       RETURN
       END
C
       SUBROUTINE RESET_INTEG_STATS()
       IMPLICIT NONE
       INCLUDE 'integ_stats.cmn'
       INTEG_NCHAMP = 0
       INTEG_NSTEPS = 0
       RETURN
       END
C
       SUBROUTINE GET_INTEG_STATS(nchamp, nsteps)
       IMPLICIT NONE
       INTEGER*8 nchamp, nsteps
       INCLUDE 'integ_stats.cmn'
       nchamp = INTEG_NCHAMP
       nsteps = INTEG_NSTEPS
       RETURN
       END
C
C***********************************************************************
C* Accessor used by the C stepper (integrators_c.c) so it never
C* touches the INTEG_STATS common block's memory layout directly -
C* only Fortran code reads/writes that common.
C***********************************************************************
       SUBROUTINE INTEG_BUMP(nchamp, nsteps)
       IMPLICIT NONE
       INTEGER*4 nchamp, nsteps
       INCLUDE 'integ_stats.cmn'
       INTEG_NCHAMP = INTEG_NCHAMP + nchamp
       INTEG_NSTEPS = INTEG_NSTEPS + nsteps
       RETURN
       END
C
C***********************************************************************
C* FSAL cache accessors, sharing the /sksyst_fsal/ cache that
C* sksyst_rk4 (calcul_Lstar_o.f, upstream el_paso patch) already
C* populates and that sksyst_reset already invalidates on field-state
C* changes. Only one INTEG_METHOD runs per trace, so any stepper can
C* safely read and populate the same cache - CHAMP is a pure function
C* of position for a fixed field state, so a hit from a different call
C* site is still the correct value, never a stale approximation.
C***********************************************************************
       SUBROUTINE FSAL_GET(x, hit, B, Bl)
       IMPLICIT NONE
       REAL*8 x(3), B(3), Bl
       INTEGER*4 hit
       REAL*8 ckx(3),ckB(3),ckBl
       INTEGER*4 ckvalid
       COMMON /sksyst_fsal/ckx,ckB,ckBl,ckvalid
       IF (ckvalid.EQ.1 .AND. x(1).EQ.ckx(1) .AND. x(2).EQ.ckx(2)
     &     .AND. x(3).EQ.ckx(3)) THEN
          hit = 1
          B(1) = ckB(1)
          B(2) = ckB(2)
          B(3) = ckB(3)
          Bl = ckBl
       ELSE
          hit = 0
       ENDIF
       RETURN
       END
C
       SUBROUTINE FSAL_SET(x, B, Bl)
       IMPLICIT NONE
       REAL*8 x(3), B(3), Bl
       REAL*8 ckx(3),ckB(3),ckBl
       INTEGER*4 ckvalid
       COMMON /sksyst_fsal/ckx,ckB,ckBl,ckvalid
       ckx(1) = x(1)
       ckx(2) = x(2)
       ckx(3) = x(3)
       ckB(1) = B(1)
       ckB(2) = B(2)
       ckB(3) = B(3)
       ckBl = Bl
       ckvalid = 1
       RETURN
       END
C
       SUBROUTINE FSAL_INVALIDATE()
       IMPLICIT NONE
       REAL*8 ckx(3),ckB(3),ckBl
       INTEGER*4 ckvalid
       COMMON /sksyst_fsal/ckx,ckB,ckBl,ckvalid
       ckvalid = 0
       RETURN
       END
C
C***********************************************************************
C* Dispatcher: replaces the historical single-body sksyst. Method 0
C* calls sksyst_rk4, an exact copy of the original routine, so default
C* behavior (INTEG_METHOD left at 0) is bit-for-bit unchanged. Method 1
C* (sksyst_rk3) is implemented in C - see integrators_c.c - kept here
C* only as an EXTERNAL declaration so this file's build doesn't depend
C* on link order.
C***********************************************************************
       SUBROUTINE sksyst(h,xx,x2,Bl,Ifail)
       IMPLICIT NONE
       INTEGER*4 Ifail
       REAL*8 xx(3),x2(3)
       REAL*8 Bl
       REAL*8 h
       INCLUDE 'integ_sel.cmn'
       EXTERNAL sksyst_rk3
C
       IF (INTEG_METHOD.EQ.1) THEN
          CALL sksyst_rk3(h,xx,x2,Bl,Ifail)
       ELSE
          CALL sksyst_rk4(h,xx,x2,Bl,Ifail)
       ENDIF
       RETURN
       END
