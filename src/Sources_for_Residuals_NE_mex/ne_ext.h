
 /**
 * \file ne_ext.c
 * \author Oscar Gerelli
 * \author Corrado Guarino Lo Bianco
 * \brief Compute the extended recursive Newton-Euler algorithm
 */

#ifndef _ne_ext_h_
#define _ne_ext_h_

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

void newton_euler_ext (
	Robot	*robot,		/*!< robot object  */
	double	*tau,		/*!< returned joint torques */
	double	*qd,		/*!< joint velocities */
	double	*qdd,		/*!< joint accelerations */
    double  *qddd,      /*!< joint jerks */
	double	*fext,		/*!< external force on manipulator tip */
	int	stride		    /*!< indexing stride for qd, qdd */
); 


#endif
