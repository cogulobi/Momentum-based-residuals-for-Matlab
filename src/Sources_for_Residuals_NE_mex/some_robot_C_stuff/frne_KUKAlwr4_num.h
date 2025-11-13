
/**
  * @file frne_KUKAlwr4_num.h
  * @brief Numerical calculus of the Newton-Euler algorithm for a Kuka LWR IV Robot
  *
  * @author Giammarco Tonti
  * @author Corrado Guarino Lo Bianco
  * @date 2025-11-10
  */

#ifndef	_frne_kukalwr_num_h_
#define	_frne_kukalwr_num_h_

#include <math.h>

void newton_euler_num_DH(double *Tau, double *q, double *qd, double *qdd);

void newton_euler_num_DH_grav(double *Tau, double *q, double *qd, double *qdd, double *grav, double* S, double* C);

void newton_euler_num_MDH(double *Tau, double *q, double *qd, double *qdd);

void newton_euler_num_MDH_grav(double *Tau, double *q, double *qd, double *qdd, double *grav);


#endif
