
/**
 * @file dotMdotq_KUKAlwr4_num.h
 * @brief Numerical calculus of the term dotM*dotq for a Kuka LWR IV Robot
 *
 * @author Giammarco Tonti
 * @author Corrado Guarino Lo Bianco
 * @date 2025-11-10
 */

#ifndef	_dotMdotq_KukaLWR4_num_h_
#define	_dotMdotq_KukaLWR4_num_h_

#include <math.h>

/* Cited paper: 
 * J. Wu, Z. Liao, Y. Han, and Z. Xiong, “An Integral Design for High Performance Sensor-less Collision Detection of Serial Robots,” 
 * in 2022 IEEE/ASME International Conference on Advanced Intelligent Mechatronics (AIM), 2022, pp. 1714–1721.
*/

void dotMdotq_num_DH_KUKAlwr4_num_reduced(double *dTau, double *q, double *qd, double *qdd, double* S, double* C);


#endif