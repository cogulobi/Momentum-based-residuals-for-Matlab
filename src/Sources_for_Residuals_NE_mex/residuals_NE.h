
/**
 * @file residuals_NE.h
 * @brief Definitions for MEX file
 *
 * @author Giammarco Tonti
 * @author Corrado Guarino Lo Bianco
 * @date 2025-11-10
 */

#ifndef	_RES_CALCULUS_h_
#define	_RES_CALCULUS_h_

#include	<math.h>
#include 	<string.h>

#include	"vmath_ext.h"

#include 	"mex.h"

#define	TRUE	1
#define	FALSE	0

/*
 * Accessing information within a MATLAB structure is inconvenient and slow.
 * To get around this we build our own robot and link data structures, and
 * copy the information from the MATLAB objects once per call.  If the call
 * is for multiple states values then our efficiency becomes very high.
 */

/* Robot kinematic convention */
typedef
	enum _dhtype {
		STANDARD,
		MODIFIED
} DHType;

/* Link joint type */
typedef
	enum _axistype {
		REVOLUTE = 0,
		PRISMATIC = 1
} Sigma;

/* A robot link structure */
typedef struct _link {
	/**********************************************************
	 *************** kinematic parameters *********************
	 **********************************************************/
	double	alpha;		/* link twist */
	double	A;		    /* link offset */
	double	D;		    /* link length */
	double	theta;		/* link rotation angle */
	double	offset;		/* link coordinate offset */
	int	sigma; 		    /* axis type; revolute or prismatic */

	/**********************************************************
	 ***************** dynamic parameters *********************
	 **********************************************************/

	/**************** of links ********************************/
	Vect	*rbar;		/* centre of mass of link wrt link origin */
	double	m;		    /* mass of link */
	double	*I;		    /* inertia tensor of link wrt link origin */

	/**************** of actuators *****************************/
		/* these parameters are motor referenced */
	double	Jm;		   /* actuator inertia */
	double	G;		   /* gear ratio */
	double	B;		   /* actuator friction damping coefficient */
	double	*Tc;	   /* actuator Coulomb friction coeffient */

	/**********************************************************
	 **************** intermediate variables ******************
	 **********************************************************/
	Vect	r;		    /* distance of ith origin from i-1th wrt ith */
	Rot  	R;		    /* link rotation matrix */
	Vect	omega;		/* angular velocity */
	Vect	omega_d;	/* angular acceleration */
    Vect    omega_dd;   /* angular jerk */
	Vect	acc;		/* acceleration */
    Vect    jerk;       /* linear jerk */
	Vect	abar;		/* acceleration of centre of mass */
    Vect    jcog;       /* linear jerk of centre of mass */
	Vect	f;		    /* inter-link force */
    Vect    fd;         /* inter-link force derivative */
	Vect	n;		    /* inter-link moment */
    Vect    nd;         /* inter-link moment derivative */
} Link;

/* A robot */
typedef struct _robot {
	int	njoints;	    /* number of joints */
	Vect	*gravity;	/* gravity vector */
	DHType	dhtype;		/* kinematic convention */
	Link	*links;		/* the links */
} Robot;

#endif

