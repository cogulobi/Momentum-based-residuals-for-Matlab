
/**
 * @file frne_ext_KUKAlwr4_num.h
 * @brief Numerical calculus of the Extended Newton-Euler algorithm for a Kuka LWR IV Robot
 *
 * @author Giammarco Tonti
 * @author Corrado Guarino Lo Bianco
 * @date 2025-11-10
 */

#ifndef	_frne_ext_kukalwr_num_h_
#define	_frne_ext_kukalwr_num_h_

#include <math.h>

void newton_euler_ext_num_DH(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd);

void newton_euler_ext_num_DH_grav(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double *grav, double* S, double* C);

void newton_euler_ext_num_DH_grav_opt(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double *grav, double* S, double* C);

void newton_euler_ext_num_MDH_grav_opt(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double *grav, double* S, double* C);


#endif