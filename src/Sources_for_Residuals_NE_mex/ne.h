/**
 * \file ne.h
 * \author Peter Corke
 * \brief Definitions for MEX file
 *
 */

#ifndef _ne_h_
#define _ne_h_

#include    <math.h>
#include	"residuals_NE.h"
#include	"vmath_ext.h"

/**
 * Recursive Newton-Euler algorithm.
 *
 * @Note the parameter \p stride which is used to allow for input and output
 * arrays which are 2-dimensional but in column-major (Matlab) order.  We
 * need to access rows from the arrays.
 *
 */
void newton_euler (
	Robot	*robot,		/*!< robot object  */
	double	*tau,		/*!< returned joint torques */
	double	*qd,		/*!< joint velocities */
	double	*qdd,		/*!< joint accelerations */
	double	*fext,		/*!< external force on manipulator tip */
	int	stride		/*!< indexing stride for qd, qdd */
);


#endif