
/**
  * @file frne_PlanarN_num.h
  * @brief Numerical calculus of the Newton-Euler algorithm for a N-link Planar Robot
  *
  * @author Giammarco Tonti
  * @author Corrado Guarino Lo Bianco
  * @date 2025-11-10
  */

#ifndef	_frne_planarn_num_h_
#define	_frne_planarn_num_h_

#include <math.h>

// Simplified planar dynamic parameters

void newton_euler_num_DH_planar3(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar4(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar5(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar6(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar7(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar8(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar9(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar10(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar11(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar12(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_DH_planar13(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);


#endif
