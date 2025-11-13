
/**
 * @file dotMdotq_PlanarN_num.h
 * @brief Numerical calculus of the term dotM*dotq for a N-link Planar Robot
 *
 * @author Giammarco Tonti
 * @author Corrado Guarino Lo Bianco
 * @date 2025-11-10
 */

#ifndef	_dotMdotq_PlanarN_num_h_
#define	_dotMdotq_PlanarN_num_h_

#include <math.h>

/* Cited paper: 
 * J. Wu, Z. Liao, Y. Han, and Z. Xiong, “An Integral Design for High Performance Sensor-less Collision Detection of Serial Robots,” 
 * in 2022 IEEE/ASME International Conference on Advanced Intelligent Mechatronics (AIM), 2022, pp. 1714–1721.
*/

// Simplified planar dynamic parameters

void dotMdotq_num_DH_planar3(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar4(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar5(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar6(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar7(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar8(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar9(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar10(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar11(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar12(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);

void dotMdotq_num_DH_planar13(double *Tau, double *dTau, double *q, double *qd, double *qdd, double *qddd, double* S, double* C);


#endif