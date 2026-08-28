/***************************************************************************
 * integrators_c.c
 *
 * C implementation of the non-default field-line stepper dispatched
 * from source/integrators_bench.f (sksyst_rk3). The legacy fixed-step
 * RK4 (sksyst_rk4) stays untouched Fortran in calcul_Lstar_o.f - it is
 * the correctness baseline and INTEG_METHOD's default, so there is no
 * reason to disturb it.
 *
 * Fortran interop notes (see source/idl_wrappers.c for the existing
 * precedent this mirrors):
 *   - This Makefile compiles Fortran with -fno-second-underscore, so
 *     every external Fortran name is exposed as lowercase + exactly
 *     one trailing underscore. Functions here are named to match
 *     (sksyst_rk3_) so Fortran's "CALL sksyst_rk3(...)" resolves to
 *     them directly.
 *   - Fortran passes every argument by reference, so every parameter
 *     here is a pointer: REAL*8 -> double*, REAL*8 xx(3) -> double*
 *     (3 contiguous doubles), INTEGER*4 -> int*.
 *   - Shared state (INTEG_METHOD, the FSAL cache, the CHAMP/step
 *     counters) lives in Fortran COMMON blocks. Rather than mirror
 *     their memory layout here (fragile: Fortran commons are packed
 *     with no padding, so a C struct guess can silently misalign),
 *     this file only talks to that state through small Fortran
 *     accessor subroutines - fsal_get_/fsal_set_/fsal_invalidate_ and
 *     integ_bump_, all defined in integrators_bench.f.
 ***************************************************************************/

/* CHAMP: the Fortran magnetic-field evaluator every stepper calls. */
extern void champ_(double *xGEO, double *B, double *Bl, int *Ifail);

/* Shared-state accessor (Fortran, in integrators_bench.f). */
extern void integ_bump_(int *nchamp, int *nsteps);

/* FSAL cache (Fortran, in integrators_bench.f) - shares the same
 * /sksyst_fsal/ cache sksyst_rk4 already populates. See that file's
 * comment for why a hit is always correct, never a stale guess. */
extern void fsal_get_(double *x, int *hit, double *B, double *Bl);
extern void fsal_set_(double *x, double *B, double *Bl);
extern void fsal_invalidate_(void);

static void vec_axpy(double dst[3], const double base[3],
                      double a, const double x[3]) {
    /* dst = base + a*x */
    dst[0] = base[0] + a * x[0];
    dst[1] = base[1] + a * x[1];
    dst[2] = base[2] + a * x[2];
}

/* Evaluate the field-line ODE's right-hand side at x, scaled to the
 * arc-length step h: k = h * B(x)/|B(x)|. Returns |B(x)| via *Bl and
 * the CHAMP failure code via *Ifail. Bumps *nchamp on every call. */
static void eval_stage(const double x[3], double h, double k[3],
                        double *Bl, int *Ifail, int *nchamp) {
    double B[3];
    double xloc[3];
    xloc[0] = x[0]; xloc[1] = x[1]; xloc[2] = x[2];
    champ_(xloc, B, Bl, Ifail);
    (*nchamp)++;
    if (*Ifail < 0) return;
    k[0] = h * B[0] / *Bl;
    k[1] = h * B[1] / *Bl;
    k[2] = h * B[2] / *Bl;
}

/* Stage 1, FSAL-aware: xx is where the PREVIOUS call in this trace
 * left off (the outer loop always does x1 = x2 before the next sksyst
 * call), so it is very likely the point that call's final "report Bl
 * at the endpoint" already evaluated and cached. Only worth checking
 * here, at stage 1 - later stages sit at freshly-computed intermediate
 * positions that essentially never recur. */
static void eval_stage1_fsal(const double x[3], double h, double k[3],
                              double *Bl, int *Ifail, int *nchamp) {
    double B[3];
    double xloc[3];
    int hit;
    xloc[0] = x[0]; xloc[1] = x[1]; xloc[2] = x[2];
    fsal_get_(xloc, &hit, B, Bl);
    if (hit) {
        *Ifail = 0;
    } else {
        champ_(xloc, B, Bl, Ifail);
        (*nchamp)++;
        if (*Ifail < 0) return;
    }
    k[0] = h * B[0] / *Bl;
    k[1] = h * B[1] / *Bl;
    k[2] = h * B[2] / *Bl;
}

/* Final "report Bl at the endpoint" call every stepper makes to match
 * sksyst's convention of returning field data at x2. Always a real
 * CHAMP call (this position is new), but its result is cached for the
 * NEXT call's stage 1 - same FSAL trick sksyst_rk4 already uses. */
static void eval_endpoint_fsal(const double x2[3], double *Bl,
                                int *Ifail, int *nchamp) {
    double B[3];
    double xloc[3];
    xloc[0] = x2[0]; xloc[1] = x2[1]; xloc[2] = x2[2];
    champ_(xloc, B, Bl, Ifail);
    (*nchamp)++;
    if (*Ifail < 0) {
        fsal_invalidate_();
        return;
    }
    fsal_set_(xloc, B, Bl);
}

/***************************************************************************
 * sksyst_rk3: fixed-step classical (Kutta) third-order Runge-Kutta.
 * 3 CHAMP evaluations for the stages + 1 to report Bl at the new
 * point (matching sksyst_rk4's convention of returning field data at
 * x2), vs. 5 total CHAMP calls in sksyst_rk4 - or, with the FSAL cache
 * both share, 3 calls/step steady-state vs RK4's 4.
 ***************************************************************************/
void sksyst_rk3_(double *h, double *xx, double *x2, double *Bl,
                  int *Ifail) {
    double k1[3], k2[3], k3[3], xa[3], xb[3];
    int nchamp = 0, nsteps = 0;

    eval_stage1_fsal(xx, *h, k1, Bl, Ifail, &nchamp);
    if (*Ifail < 0) { integ_bump_(&nchamp, &nsteps); return; }
    vec_axpy(xa, xx, 0.5, k1);

    eval_stage(xa, *h, k2, Bl, Ifail, &nchamp);
    if (*Ifail < 0) { integ_bump_(&nchamp, &nsteps); return; }
    /* xb = xx - k1 + 2*k2 */
    xb[0] = xx[0] - k1[0] + 2.0 * k2[0];
    xb[1] = xx[1] - k1[1] + 2.0 * k2[1];
    xb[2] = xx[2] - k1[2] + 2.0 * k2[2];

    eval_stage(xb, *h, k3, Bl, Ifail, &nchamp);
    if (*Ifail < 0) { integ_bump_(&nchamp, &nsteps); return; }
    x2[0] = xx[0] + (k1[0] + 4.0 * k2[0] + k3[0]) / 6.0;
    x2[1] = xx[1] + (k1[1] + 4.0 * k2[1] + k3[1]) / 6.0;
    x2[2] = xx[2] + (k1[2] + 4.0 * k2[2] + k3[2]) / 6.0;

    eval_endpoint_fsal(x2, Bl, Ifail, &nchamp);
    nsteps++;
    integ_bump_(&nchamp, &nsteps);
}
