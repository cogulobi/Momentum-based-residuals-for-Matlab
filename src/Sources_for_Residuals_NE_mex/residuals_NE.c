
/**
 * @file residuals_NE.c
 * @brief MEX file body
 *
 * @author Giammarco Tonti
 * @author Corrado Guarino Lo Bianco
 * @date 2025-11-10
 */

/*
 *
 *  RESIDUALS_NE	 MEX file for the evaluation of RESIDUALS TERMS for a Kuka LWR IV or a N-links planar manipulator.
 *
 *	[OUTER_TERM, INNER_TERM, COMP_TIMING] = RESIDUALS_NE(ROBOT, Q, QD, QDD, QDDD, FEXT, RES_CALCULUS_MODE, IS_PLANAR_ROBOT)
 *
 *	where:
 *  -	Q, QD, QDD, and QDDD are row vectors of the manipulator state: pos, vel, accel, and jerk;
 *  - 	FEXT is the external force/moment acting on the end of the manipulator. It may also be 
 * 		specified by a 6-element vector [Fx Fy Fz Mx My Mz];
 *  -	RES_CALCULUS_MODE determines how the residuals are going to be evaluated:
 * 		    0 -> just General-Purpose NE function calls will be used (NE);
 * 			1 -> both General-Purpose NE and extended NE function calls will be used (Ext. NE);
 * 			2 -> just Numerical NE function calls will be used (Cust. NE);
 * 			3 -> both Numerical NE and extended RNE function calls will be used (Cust. Ext. NE);
 * 			4 -> classical Euler-Lagrange method. M, S and g are computed numerically (Cust. EL);
 * 			5 -> dotM*dotq in addiction to Numerical NE function calls (Paper [23] of the article).
 *  -   IS_PLANAR_ROBOT determines which robot is passed as variable to "residuals_NE(...)" function:
 * 			0 -> KUKA LWR IV robot;
 * 			1 -> N-LINK PLANAR robot.
 *
 * 	Returns 2 row vectors, 1 by N (N is the number of robot joints), which correspond to the terms that charaterize the MOMENTUM-BASED RESIDUALS computation,
 *  and 1 value which represents how much time "residuals_NE(...)" takes to compute it:
 *  	OUTER_TERM:  robot generalized momentum -> M(Q)*QD;
 * 		INNER_TERM:  summation of all the terms within the integral (without friction terms);
 * 		COMP_TIMING: how much time the residual calculus takes.
 *  
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "ne.h"
#include "ne_ext.h"
#include "residuals_NE.h"

// KUKA LWR IV
#include "some_robot_C_stuff/frne_KUKAlwr4_num.h"
#include "some_robot_C_stuff/frne_ext_KUKAlwr4_num.h"

#include "some_robot_C_stuff/get_KukaMnum.h"
#include "some_robot_C_stuff/get_KukaSnum.h"
#include "some_robot_C_stuff/get_Kukagnum.h"

#include "some_robot_C_stuff/dotMdotq_KUKAlwr4_num.h"

// N-LINK PLANAR ROBOT
#include "some_robot_C_stuff/frne_PlanarN_num.h"
#include "some_robot_C_stuff/frne_ext_PlanarN_num.h"

#include "some_robot_C_stuff/get_PlanarN_3_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_3_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_4_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_4_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_5_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_5_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_6_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_6_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_7_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_7_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_8_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_8_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_9_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_9_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_10_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_10_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_11_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_11_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_12_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_12_Snum.h"
#include "some_robot_C_stuff/get_PlanarN_13_Mnum.h"
#include "some_robot_C_stuff/get_PlanarN_13_Snum.h"

#include "some_robot_C_stuff/dotMdotq_PlanarN_num.h"

#include "mex.h"

// #define LINUX_MACHINE

#ifdef LINUX_MACHINE

#include "time.h"

double ts_to_ns(const struct timespec *ts) 
{
	return (double)(ts->tv_sec*1E9) + (double)(ts->tv_nsec);
} 

#else

#include <windows.h>

#endif

// #define DEBUG
// #define RESIDUAL_TERMS_DEBUG

/* Input Arguments */
#define	ROBOT_IN	prhs[0]
#define	A1_IN		prhs[1]
#define	A2_IN		prhs[2]
#define	A3_IN		prhs[3]
#define	A4_IN		prhs[4]
#define	A5_IN		prhs[5]
#define A6_IN       prhs[6]
#define A7_IN       prhs[7]

/* Output Arguments */
#define	OUTER_TERM     plhs[0]
#define	INNER_TERM     plhs[1]
#define COMP_TIMING	   plhs[2]

/* Some useful things */
#define	NUMROWS(x)	mxGetM(x)
#define	NUMCOLS(x)	mxGetN(x)
#define	NUMELS(x)   (mxGetN(x)*mxGetM(x))
#define	POINTER(x)	mxGetPr(x)

/* forward defines */
static void rot_mat (Link *l, double th, double d, DHType type);
static int mstruct_getfield_number(mxArray *m, char *field);
static int mstruct_getint(mxArray *m, int i, char *field);
static double mstruct_getreal(mxArray *m, int i, char *field);
static double * mstruct_getrealvect(mxArray *m, int i, char *field);
void error(char *s, ...);

void coriolis_Paper(Robot *robot, double *q, double *qd, int njoints, double **C_out);
void coriolis_Paper_num(double *q, double *qd, int njoints, double **C_out, double *S_in, double *C_in, int is_planar_robot);

/* default values for gravity and external load */

/**
 * MEX function entry point.
 */
void 
mexFunction(
	int		nlhs,
	mxArray	*plhs[],
	int		nrhs,
	const mxArray	*prhs[]
) {
    double      zeroV[2] = {0.0, 0.0};
	double      *q, *qd, *qdd, *qddd;
	double      *tau;
	unsigned int	m,n;
	int         j, njoints, p, nq;
	double      *fext = NULL;
	double      *grav = NULL;
    char        *frict = NULL;
	Robot		robot;
	mxArray		*link0;
	mxArray		*mx_robot;
	mxArray		*mx_links;
	static int	firstime = 0;

	double 		*inner_res_term, *outer_res_term, *comp_timing_term;
    int         res_calculus_mode, is_planar_robot;
	double 		*null_vec_nJoints, *cumulative_res_term;
	
	if ( !mxIsClass(ROBOT_IN, "SerialLink") )
		mexErrMsgTxt("(residuals_NE): first argument is not a robot structure\n");
	
    mx_robot = (mxArray *)ROBOT_IN;
	njoints = mstruct_getint(mx_robot, 0, "n");

	//double C_out[njoints][njoints];

/***********************************************************************
 * Handle the different calling formats.
 * Setup pointers to q, qd and qdd inputs 
 ***********************************************************************/
	switch (nrhs) {

    case 8: {
	/*
	 * [OUTER_TERM, INNER_TERM, COMP_TIMING] = RESIDUALS_NE(ROBOT, Q, QD, QDD, QDDD, FEXT, RES_CALCULUS_MODE, IS_PLANAR_ROBOT)
	 */
		int	nqd = NUMROWS(A2_IN),
			nqdd = NUMROWS(A3_IN),
            nqddd = NUMROWS(A4_IN);

		nq = NUMROWS(A1_IN);
		if ((nq != nqd) || (nqd != nqdd) || (nqdd != nqddd))
			mexErrMsgTxt("(residuals_NE): Q QD QDD QDDD must have the same length");
		if ( (NUMCOLS(A1_IN) != njoints) ||
		     (NUMCOLS(A2_IN) != njoints) ||
		     (NUMCOLS(A3_IN) != njoints) ||
             (NUMCOLS(A4_IN) != njoints)
		) 
			mexErrMsgTxt("(residuals_NE): Q must have Naxis columns");
		q = POINTER(A1_IN);
		qd = POINTER(A2_IN);
		qdd = POINTER(A3_IN);
        qddd = POINTER(A4_IN);
		if (NUMELS(A5_IN) != 6)
			mexErrMsgTxt("(residuals_NE): Fext vector expected");
		fext = POINTER(A5_IN);
        res_calculus_mode = (int) mxGetScalar(A6_IN);
		is_planar_robot = (int) mxGetScalar(A7_IN);
		break;
	}
	default:
		mexErrMsgTxt("(residuals_NE): wrong number of arguments.");
	}
	
	/*
	 * fill out the robot structure
	 */
	robot.njoints = njoints;

	if (grav)
		robot.gravity = (Vect *)grav;
	else
		robot.gravity = (Vect *)mxGetPr( mxGetProperty(mx_robot, (mwIndex)0, "gravity") );
	robot.dhtype = mstruct_getint(mx_robot, 0, "mdh");

	/* build link structure */
	robot.links = (Link *)mxCalloc((mwSize) njoints, (mwSize) sizeof(Link));

	if (firstime == 0) {
		mexPrintf("(residuals_NE): (c) Giammarco Tonti & Corrado Guarino Lo Bianco 2025\n");
		firstime = 1;
	}

/***********************************************************************
 * Now we have to get pointers to data spread all across a cell-array
 * of Matlab structures.
 *
 * Matlab structure elements can be found by name (slow) or by number (fast).
 * We assume that since the link structures are all created by the same
 * constructor, the index number for each element will be the same for all
 * links.  However we make no assumption about the numbers themselves.
 ***********************************************************************/
    
	/* get pointer to the first link structure */
	link0 = mxGetProperty(mx_robot, (mwIndex) 0, "links");
	if (link0 == NULL)
		mexErrMsgTxt("couldnt find element link in robot structure");

	/*
	 * Elements of the link structure are:
	 *
	 *	alpha: 
	 *	A:
	 *	theta:
	 *	D:
	 *	offset:
	 *	sigma:
	 *	mdh:
	 *	m:
	 *	r:
	 *	I:
	 *	Jm:
	 *	G:
	 *	B:
	 *	Tc:
	 */

	/*
     * copy data from the Link objects into the local links structure
     * to save function calls later
     */

    for (j=0; j<njoints; j++) {
        Link    *l = &robot.links[j];
        mxArray *links = mxGetProperty(mx_robot, (mwIndex) 0, "links"); /* links array */

        l->alpha =  mxGetScalar( mxGetProperty(links, (mwIndex) j, "alpha") );
        l->A =      mxGetScalar( mxGetProperty(links, (mwIndex) j, "a") );
        l->theta =  mxGetScalar( mxGetProperty(links, (mwIndex) j, "theta") );
        l->D =      mxGetScalar( mxGetProperty(links, (mwIndex) j, "d") );

		//l->sigma =  mxGetScalar( mxGetProperty(links, (mwIndex) j, "jointtype") ); // "sigma"
        char *jointtype = mxArrayToString( mxGetProperty(links, (mwIndex) j, "jointtype") );
        if (strcmp(jointtype, "R") == 0) 
            l->sigma = 0; // Revolute joint
        else
            l->sigma = 1; // Prismatic joint

        l->offset = mxGetScalar( mxGetProperty(links, (mwIndex) j, "offset") );
        l->m =      mxGetScalar( mxGetProperty(links, (mwIndex) j, "m") );
        l->rbar =   (Vect *)mxGetPr( mxGetProperty(links, (mwIndex) j, "r") );
        l->I =      mxGetPr( mxGetProperty(links, (mwIndex) j, "I") );
        l->Jm =     mxGetScalar( mxGetProperty(links, (mwIndex) j, "Jm") );
        l->G =      mxGetScalar( mxGetProperty(links, (mwIndex) j, "G") );
        if (frict == NULL || strcmp(frict, "v") == 0 || strcmp(frict, "b") == 0) 
		  l->B = mxGetScalar( mxGetProperty(links, (mwIndex) j, "B") );
        else
		  l->B = 0.0;
		if (frict == NULL || strcmp(frict, "c") == 0 || strcmp(frict, "b") == 0)
		  l->Tc = mxGetPr( mxGetProperty(links, (mwIndex) j, "Tc") );
        else
		  l->Tc = zeroV;    
    }
    
	/* build link structure */
	tau = (double *)mxCalloc((mwSize) 2*njoints, (mwSize) sizeof(double));
	
	null_vec_nJoints = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	cumulative_res_term = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));

	// Residual calculus variables initialization
	// Coriolis matrix C(q,qdot)
	double *term_1 = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double **C_out = (double **)mxCalloc((mwSize) njoints, (mwSize) sizeof(double *));
	double **M = (double **)mxCalloc((mwSize) njoints, (mwSize) sizeof(double *));
	double **S = (double **)mxCalloc((mwSize) njoints, (mwSize) sizeof(double *));
	for (int k = 0; k < njoints; k++) {
		C_out[k] = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
		M[k] = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
		S[k] = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	}
	double *M_vec = (double *)mxCalloc((mwSize) njoints*njoints, (mwSize) sizeof(double));
	double *S_vec = (double *)mxCalloc((mwSize) njoints*njoints, (mwSize) sizeof(double));
	double *g_vec = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	// Numerical routine valid only when it is considering KUKA LWR IV
	double *tmp_tau = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double *tmp_dTau = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double *tmp_q = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double *tmp_qd = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double *tmp_qdd = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double *tmp_qddd = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double *S_robot = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double *C_robot = (double *)mxCalloc((mwSize) njoints, (mwSize) sizeof(double));
	double g_acc[3];

	/* Create a matrix for the return arguments */
	INNER_TERM = mxCreateDoubleMatrix((mwSize) nq, (mwSize) njoints, mxREAL);
	inner_res_term = mxGetPr(INNER_TERM);    
	OUTER_TERM = mxCreateDoubleMatrix((mwSize) nq, (mwSize) njoints, mxREAL);
	outer_res_term = mxGetPr(OUTER_TERM); 
	COMP_TIMING = mxCreateDoubleMatrix((mwSize) nq, (mwSize) 1, mxREAL);
	comp_timing_term = mxGetPr(COMP_TIMING);    
	for (int k = 0; k < njoints; k++) {
		inner_res_term[k] = 0;
		outer_res_term[k] = 0;

		null_vec_nJoints[k] = 0;
		cumulative_res_term[k] = 0;
	}
	for (int k = 0; k < nq; k++) {
		comp_timing_term[k] = 0;
	}

#define	MEL(x,R,C)	(x[(R)+(C)*nq])


	/* for each point in the input trajectory */
	for (p=0; p<nq; p++) {

		/******************************************/
		/******** START RESIDUALS CALCULUS ********/
		/******************************************/

		// Start measuring the excution time
		double elapsed_time_usec;
	#ifdef LINUX_MACHINE

		struct timespec start, end;
		clock_gettime(CLOCK_MONOTONIC, &start);

	#else

		LARGE_INTEGER frequency, start, end;
		// Get the high resolution counter frequency
		QueryPerformanceFrequency(&frequency);
		// Start measuring the excution time
		QueryPerformanceCounter(&start);

	#endif

#ifdef RESIDUAL_TERMS_DEBUG
		mexPrintf("\nParameter Value 'res_calculus_mode': %d", res_calculus_mode);
		mexPrintf("\nParameter Value 'is_planar_robot': %d", is_planar_robot);
#endif
		switch (res_calculus_mode) {
			/*****************************************************************************************************/
			/****** Calculation of residuals using just General-Purpose NE function calls will be used (NE) ******/
			/*****************************************************************************************************/
			case 0:

				// update all position dependent variables
				for (j = 0; j < njoints; j++) {
					Link	*l = &robot.links[j];

					switch (l->sigma) {
					case REVOLUTE:
						rot_mat(l, MEL(q,p,j)+l->offset, l->D, robot.dhtype);
						break;
					case PRISMATIC:
						rot_mat(l, l->theta, MEL(q,p,j)+l->offset, robot.dhtype);
						break;
					}
	#ifdef	DEBUG
					rot_print("R", &l->R);
					vect_print("p*", &l->r);
	#endif
				}

	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n**********************************************************************************");
				mexPrintf("\n*** Calculation of residuals using just General-Purpose NE function calls (NE) ***");
				mexPrintf("\n**********************************************************************************\n");
	#endif
				// Outer term -> M(q)*qdot
				robot.gravity->x = 0; robot.gravity->y = 0; robot.gravity->z = 0;
				newton_euler(&robot, tau, null_vec_nJoints, &qd[p], fext, nq);
				for (int k = 0; k < njoints; k++)
					outer_res_term[p+k] = tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Outer Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", outer_res_term[k]);
				mexPrintf("\n");
	#endif
				// Inner Term 2 -> g(q)
				robot.gravity->x = 0; robot.gravity->y = 0; robot.gravity->z = 9.80665;
				newton_euler(&robot, tau, null_vec_nJoints, null_vec_nJoints, fext, nq);
				for (int k = 0; k < njoints; k++) 
					cumulative_res_term[k] = cumulative_res_term[k] - tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 2: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k]);
				mexPrintf("\n");
	#endif
				// Inner Term 1 -> transpose( S(q,qdot) )*qdot
				coriolis_Paper(&robot, q, &qd[p], njoints, C_out);
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("C(q,qdot) matrix: \n");
				for (int k = 0; k < njoints; k++) {
					for (int l = 0; l < njoints; l++)
						mexPrintf("%10.5f ", C_out[k][l]);
					mexPrintf("\n");
				}
	#endif
				for (int i = 0; i < njoints; i++) {
					double tmp_val = 0;
					for (int j = 0; j < njoints; j++) 
						tmp_val += C_out[j][i]*qd[p+j];
					term_1[i] = tmp_val;
				}
				for (int k = 0; k < njoints; k++) {
					cumulative_res_term[k] = cumulative_res_term[k] + term_1[k];
					inner_res_term[p+k] = cumulative_res_term[k];
				}
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 1: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", term_1[k]);
				mexPrintf("\n");
	#endif
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n********************************************************************\n\n");
	#endif

			break;

			/**************************************************************************************************************************/
			/****** Calculation of residuals using both General-Purpose NE and extended NE function calls will be used (Ext. NE) ******/
			/**************************************************************************************************************************/
			case 1:
				
			    // update all position dependent variables
				for (j = 0; j < njoints; j++) {
					Link	*l = &robot.links[j];

					switch (l->sigma) {
					case REVOLUTE:
						rot_mat(l, MEL(q,p,j)+l->offset, l->D, robot.dhtype);
						break;
					case PRISMATIC:
						rot_mat(l, l->theta, MEL(q,p,j)+l->offset, robot.dhtype);
						break;
					}
	#ifdef	DEBUG
					rot_print("R", &l->R);
					vect_print("p*", &l->r);
	#endif
				}
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n*******************************************************************************************************");
				mexPrintf("\n*** Calculation of residuals using both General-Purpose NE and extended NE function calls (Ext. NE) ***");
				mexPrintf("\n*******************************************************************************************************\n");
	#endif
				// Outer term -> M(q)*qdot
				robot.gravity->x = 0; robot.gravity->y = 0; robot.gravity->z = 0;
				newton_euler(&robot, tau, null_vec_nJoints, &qd[p], fext, nq);
				for (int k = 0; k < njoints; k++)
					outer_res_term[p+k] = tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Outer Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", outer_res_term[k]);
				mexPrintf("\n");
	#endif
				// Inner Terms 1 & 3 -> NED_0(q,qdot,0,0) + 2*NE_0(q,qdot,0)
				newton_euler_ext(&robot, tau, &qd[p], null_vec_nJoints, null_vec_nJoints, fext, nq);
				for (int k = 0; k < njoints; k++) 
					cumulative_res_term[k] = cumulative_res_term[k] - tau[k+njoints] - 2.0*tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", tau[k+njoints]);
				mexPrintf("\n");
				mexPrintf("Term 3: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", (2.0*tau[k]));
				mexPrintf("\n");
	#endif
				// Inner Term 2 -> NED_0(q,qdot,qdot,0)
				newton_euler_ext(&robot, tau, &qd[p], &qd[p], null_vec_nJoints, fext, nq);
				for (int k = 0; k < njoints; k++) 
					cumulative_res_term[k]  = cumulative_res_term[k] + tau[k+njoints];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 2: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k+njoints]);
				mexPrintf("\n");
	#endif
				// Inner Term 4 -> c(q,dotq) + g(q)
				robot.gravity->x = 0; robot.gravity->y = 0; robot.gravity->z = 9.80665;
				newton_euler(&robot, tau, &qd[p], null_vec_nJoints, fext, nq);
				for (int k = 0; k < njoints; k++) {
					cumulative_res_term[k]  = cumulative_res_term[k] - tau[k];
					inner_res_term[p+k] = cumulative_res_term[k];
				}
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 4: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k]);

				mexPrintf("\n*************************************************************************\n\n");
	#endif
			break;

			/*****************************************************************************************************/
			/****** Calculation of residuals using just Numerical NE function calls will be used (Cust. NE) ******/
			/*****************************************************************************************************/
			case 2:
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n**********************************************************************************");
				mexPrintf("\n*** Calculation of residuals using just Numerical NE function calls (Cust. NE) ***");
				mexPrintf("\n**********************************************************************************\n");
	#endif

				// A bit of oprimization for Numerical routines
				for (j=0; j<njoints; j++) {
					S_robot[j] = sin(q[j]); 
					C_robot[j] = cos(q[j]);
				}

				// Outer term -> M(q)*qdot
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = null_vec_nJoints[p+j*nq];
					tmp_qdd[j] = qd[p+j*nq];
				}
				g_acc[0] = 0.0; g_acc[1] = 0.0; g_acc[2] = 0.0; //9.80665;
				if (!is_planar_robot)
					newton_euler_num_DH_grav(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_num_DH_planar3(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_num_DH_planar4(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_num_DH_planar5(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_num_DH_planar6(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_num_DH_planar7(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_num_DH_planar8(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_num_DH_planar9(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_num_DH_planar10(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_num_DH_planar11(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_num_DH_planar12(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_num_DH_planar13(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
				}

				for (int k = 0; k < njoints; k++)
					outer_res_term[p+k] = tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Outer Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", outer_res_term[k]);
				mexPrintf("\n");
	#endif

				// Inner Term 2 -> g(q)
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = null_vec_nJoints[p+j*nq];
					tmp_qdd[j] = null_vec_nJoints[p+j*nq];
				}
				g_acc[0] = 0.0; g_acc[1] = 0.0; g_acc[2] = 9.80665;
				if (!is_planar_robot)
					newton_euler_num_DH_grav(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_num_DH_planar3(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_num_DH_planar4(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_num_DH_planar5(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_num_DH_planar6(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_num_DH_planar7(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_num_DH_planar8(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_num_DH_planar9(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_num_DH_planar10(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_num_DH_planar11(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_num_DH_planar12(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_num_DH_planar13(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
				}

				for (int k = 0; k < njoints; k++) 
					cumulative_res_term[k] = cumulative_res_term[k] - tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 2: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k]);
				mexPrintf("\n");
	#endif
				// Inner Term 1 -> transpose( S(q,qdot) )*qdot
				coriolis_Paper_num(q, &qd[p], njoints, C_out, S_robot, C_robot, is_planar_robot);
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("C(q,qdot) matrix: \n");
				for (int k = 0; k < njoints; k++) {
					for (int l = 0; l < njoints; l++)
						mexPrintf("%10.5f ", C_out[k][l]);
					mexPrintf("\n");
				}
	#endif
				for (int i = 0; i < njoints; i++) {
					double tmp_val = 0;
					for (int j = 0; j < njoints; j++) 
						tmp_val += C_out[j][i]*qd[p+j];
					term_1[i] = tmp_val;
				}
				for (int k = 0; k < njoints; k++) {
					cumulative_res_term[k] = cumulative_res_term[k] + term_1[k];
					inner_res_term[p+k] = cumulative_res_term[k];
				}
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 1: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", term_1[k]);
				mexPrintf("\n");
	#endif
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n********************************************************************\n\n");
	#endif	
			break;

			/***************************************************************************************************************************/
			/****** Calculation of residuals using both Numerical NE and extended RNE function calls will be used (Cust. Ext. NE) ******/
			/***************************************************************************************************************************/
			case 3:
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n********************************************************************************************************");
				mexPrintf("\n*** Calculation of residuals using both Numerical NE and extended RNE function calls (Cust. Ext. NE) ***");
				mexPrintf("\n********************************************************************************************************\n");
	#endif

				// A bit of oprimization for Numerical routines
				for (j=0; j<njoints; j++) {
					S_robot[j] = sin(q[j]); 
					C_robot[j] = cos(q[j]);
				}

				// Outer term -> M(q)*qdot
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = null_vec_nJoints[p+j*nq];
					tmp_qdd[j] = qd[p+j*nq];
				}
				g_acc[0] = 0.0; g_acc[1] = 0.0; g_acc[2] = 0.0; //9.80665;
				if (!is_planar_robot)
					newton_euler_num_DH_grav(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_num_DH_planar3(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_num_DH_planar4(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_num_DH_planar5(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_num_DH_planar6(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_num_DH_planar7(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_num_DH_planar8(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_num_DH_planar9(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_num_DH_planar10(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_num_DH_planar11(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_num_DH_planar12(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_num_DH_planar13(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}
				
				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
				}
				
				for (int k = 0; k < njoints; k++)
					outer_res_term[p+k] = tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Outer Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", outer_res_term[k]);
				mexPrintf("\n");
	#endif

				// Inner Terms 1 & 3 -> NED_0(q,qdot,0,0) + 2*NE_0(q,qdot,0)				
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = qd[p+j*nq];
					tmp_qdd[j] = null_vec_nJoints[p+j*nq];
					tmp_qddd[j] = null_vec_nJoints[p+j*nq];
				}
				if (!is_planar_robot) {
					newton_euler_ext_num_DH_grav_opt(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
				}
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_ext_num_DH_planar3(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_ext_num_DH_planar4(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_ext_num_DH_planar5(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_ext_num_DH_planar6(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_ext_num_DH_planar7(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_ext_num_DH_planar8(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_ext_num_DH_planar9(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_ext_num_DH_planar10(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_ext_num_DH_planar11(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_ext_num_DH_planar12(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_ext_num_DH_planar13(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
					tau[p+(j+njoints)*nq] = tmp_dTau[j];
				}
				for (int k = 0; k < njoints; k++) 
					cumulative_res_term[k] = cumulative_res_term[k] - tau[k+njoints] - 2.0*tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", tau[k+njoints]);
				mexPrintf("\n");
				mexPrintf("Term 3: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", (2.0*tau[k]));
				mexPrintf("\n");
	#endif

				// Inner Term 2 -> NED_0(q,qdot,qdot,0)				
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = qd[p+j*nq];
					tmp_qdd[j] = qd[p+j*nq];
					tmp_qddd[j] = null_vec_nJoints[p+j*nq];
				}
				if (!is_planar_robot) {
					newton_euler_ext_num_DH_grav_opt(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
				}
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_ext_num_DH_planar3(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_ext_num_DH_planar4(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_ext_num_DH_planar5(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_ext_num_DH_planar6(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_ext_num_DH_planar7(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_ext_num_DH_planar8(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_ext_num_DH_planar9(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_ext_num_DH_planar10(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_ext_num_DH_planar11(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_ext_num_DH_planar12(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_ext_num_DH_planar13(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
					tau[p+(j+njoints)*nq] = tmp_dTau[j];
				}

				for (int k = 0; k < njoints; k++) 
					cumulative_res_term[k]  = cumulative_res_term[k] + tau[k+njoints];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 2: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k+njoints]);
				mexPrintf("\n");
	#endif

				// Inner Term 4 -> c(q,dotq) + g(q)
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = qd[p+j*nq];
					tmp_qdd[j] = null_vec_nJoints[p+j*nq];
				}
				g_acc[0] = 0.0; g_acc[1] = 0.0; g_acc[2] = 9.80665;
				if (!is_planar_robot)
					newton_euler_num_DH_grav(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_num_DH_planar3(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_num_DH_planar4(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_num_DH_planar5(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_num_DH_planar6(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_num_DH_planar7(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_num_DH_planar8(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_num_DH_planar9(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_num_DH_planar10(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_num_DH_planar11(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_num_DH_planar12(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_num_DH_planar13(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
				}

				for (int k = 0; k < njoints; k++) {
					cumulative_res_term[k]  = cumulative_res_term[k] - tau[k];
					inner_res_term[p+k] = cumulative_res_term[k];
				}
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 4: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k]);

				mexPrintf("\n*************************************************************************\n\n");
	#endif
				
			break;

			/***********************************************************************************************************************/
			/****** Calculation of residuals using Euler-Lagrange formulation. M, S and g are computed numerically (Cust. EL) ******/
			/***********************************************************************************************************************/
			case 4:
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n*******************************************************************************************************************");
				mexPrintf("\n**** Calculation of residuals using Euler-Lagrange formulation. M, S and g are computed numerically (Cust. EL) ****");
				mexPrintf("\n*******************************************************************************************************************\n");
	#endif		

				// A bit of oprimization for Numerical routines
				for (j=0; j<njoints; j++) {
					S_robot[j] = sin(q[j]); 
					C_robot[j] = cos(q[j]);
				}

				// Outer term -> M(q)*qdot
				if (!is_planar_robot)
					get_KukaMnum(&q[p], M_vec, S_robot, C_robot);
				else {
					switch (njoints)
					{
					case 3:
						get_PlanarN_3_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 4:
						get_PlanarN_4_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 5:
						get_PlanarN_5_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 6:
						get_PlanarN_6_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 7:
						get_PlanarN_7_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 8:
						get_PlanarN_8_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 9:
						get_PlanarN_9_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 10:
						get_PlanarN_10_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 11:
						get_PlanarN_11_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 12:
						get_PlanarN_12_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					case 13:
						get_PlanarN_13_Mnum(&q[p], M_vec, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (int i = 0; i < njoints; i++) {
					double tmp_val = 0;
					for (int j = 0; j < njoints; j++) 
						tmp_val += M_vec[j*njoints+i]*qd[p+j];
					tau[i] = tmp_val;
				}
				for (int k = 0; k < njoints; k++)
					outer_res_term[p+k] = tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Outer Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", outer_res_term[k]);
				mexPrintf("\n");
	#endif
				if (!is_planar_robot) {
					// Inner Term 2 -> g(q)
					get_Kukagnum(&q[p], g_vec, S_robot, C_robot);
					for (int k = 0; k < njoints; k++) 
						cumulative_res_term[k] = cumulative_res_term[k] - g_vec[k];
				}
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 2: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k]);
				mexPrintf("\n");
	#endif
				// Inner Term 1 -> transpose( S(q,qdot) )*qdot
				if (!is_planar_robot)
					get_KukaSnum(&q[p], &qd[p], S_vec, S_robot, C_robot);
				else {
					switch (njoints)
					{
					case 3:
						get_PlanarN_3_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 4:
						get_PlanarN_4_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 5:
						get_PlanarN_5_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 6:
						get_PlanarN_6_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 7:
						get_PlanarN_7_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 8:
						get_PlanarN_8_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 9:
						get_PlanarN_9_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 10:
						get_PlanarN_10_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 11:
						get_PlanarN_11_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 12:
						get_PlanarN_12_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					case 13:
						get_PlanarN_13_Snum(&q[p], &qd[p], S_vec, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("C(q,qdot) matrix: \n");
				for (int k = 0; k < njoints; k++) {
					for (int l = 0; l < njoints; l++)
						mexPrintf("%10.5f ", C_out[k][l]);
					mexPrintf("\n");
				}
	#endif
				for (int i = 0; i < njoints; i++) {
					double tmp_val = 0;
					for (int j = 0; j < njoints; j++) 
						tmp_val += S_vec[i*njoints+j]*qd[p+j];
					term_1[i] = tmp_val;
				}
				for (int k = 0; k < njoints; k++) {
					cumulative_res_term[k] = cumulative_res_term[k] + term_1[k];
					inner_res_term[p+k] = cumulative_res_term[k];
				}
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 1: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", term_1[k]);
				mexPrintf("\n");
	#endif
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n********************************************************************\n\n");
	#endif

			break;

			/*****************************************************************************************************************************************/
			/****** Calculation of residuals using just Numerical NE function calls and smart dotM(q)*dotq product (Paper [23] of the article)  ******/
			/*****************************************************************************************************************************************/
			/* Reference : 
			* J. Wu, Z. Liao, Y. Han, and Z. Xiong, “An Integral Design for High Performance Sensor-less Collision Detection of Serial Robots,” 
 			* in 2022 IEEE/ASME International Conference on Advanced Intelligent Mechatronics (AIM), 2022, pp. 1714–1721.
			**/
			case 5:
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n**********************************************************************************************************************************");
				mexPrintf("\n*** Calculation of residuals using just Numerical NE function calls and smart dotM(q)*dotq product (Paper [23] of the article) ***");
				mexPrintf("\n**********************************************************************************************************************************\n");
	#endif
			
				// A bit of oprimization for Numerical routines
				for (j=0; j<njoints; j++) {
					S_robot[j] = sin(q[j]); 
					C_robot[j] = cos(q[j]);
				}

				// Outer term -> M(q)*qdot
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = null_vec_nJoints[p+j*nq];
					tmp_qdd[j] = qd[p+j*nq];
				}
				g_acc[0] = 0.0; g_acc[1] = 0.0; g_acc[2] = 0.0; //9.80665;

				if (!is_planar_robot)
					newton_euler_num_DH_grav(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_num_DH_planar3(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_num_DH_planar4(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_num_DH_planar5(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_num_DH_planar6(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_num_DH_planar7(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_num_DH_planar8(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_num_DH_planar9(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_num_DH_planar10(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_num_DH_planar11(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_num_DH_planar12(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_num_DH_planar13(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
				}

				for (int k = 0; k < njoints; k++)
					outer_res_term[p+k] = tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Outer Term 1: ");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", outer_res_term[k]);
				mexPrintf("\n");
	#endif

				// Inner Term 2 -> c(q,dotq) + g(q)
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = qd[p+j*nq];
					tmp_qdd[j] = null_vec_nJoints[p+j*nq];
				}
				g_acc[0] = 0.0; g_acc[1] = 0.0; g_acc[2] = 9.80665;
				if (!is_planar_robot)
					newton_euler_num_DH_grav(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						newton_euler_num_DH_planar3(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 4:
						newton_euler_num_DH_planar4(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 5:
						newton_euler_num_DH_planar5(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 6:
						newton_euler_num_DH_planar6(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 7:
						newton_euler_num_DH_planar7(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 8:
						newton_euler_num_DH_planar8(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 9:
						newton_euler_num_DH_planar9(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 10:
						newton_euler_num_DH_planar10(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 11:
						newton_euler_num_DH_planar11(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 12:
						newton_euler_num_DH_planar12(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					case 13:
						newton_euler_num_DH_planar13(tmp_tau, tmp_q, tmp_qd, tmp_qdd, g_acc, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
				}

				for (int k = 0; k < njoints; k++) 
					cumulative_res_term[k] = cumulative_res_term[k] - tau[k];
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 2: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", tau[k]);
				mexPrintf("\n");
	#endif

				// Inner Term 1 -> dotM(q)*dotq
				for (j = 0; j < njoints; j++) {
					tmp_q[j] = q[p+j*nq];
					tmp_qd[j] = qd[p+j*nq];
					tmp_qdd[j] = qd[p+j*nq];
					tmp_qddd[j] = null_vec_nJoints[p+j*nq];
				}
				if (!is_planar_robot) {
					dotMdotq_num_DH_KUKAlwr4_num_reduced(tmp_dTau, tmp_q, tmp_qd, tmp_qdd, S_robot, C_robot);
				}
				else {
					// N-LINK PLANAR ROBOT
					switch (njoints)
					{
					case 3:
						dotMdotq_num_DH_planar3(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 4:
						dotMdotq_num_DH_planar4(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 5:
						dotMdotq_num_DH_planar5(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 6:
						dotMdotq_num_DH_planar6(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 7:
						dotMdotq_num_DH_planar7(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 8:
						dotMdotq_num_DH_planar8(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 9:
						dotMdotq_num_DH_planar9(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 10:
						dotMdotq_num_DH_planar10(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 11:
						dotMdotq_num_DH_planar11(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 12:
						dotMdotq_num_DH_planar12(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					case 13:
						dotMdotq_num_DH_planar13(tmp_tau, tmp_dTau, tmp_q, tmp_qd, tmp_qdd, tmp_qddd, S_robot, C_robot);
						break;
					default:
						break;
					}
				}

				for (j = 0; j < njoints; j++) {
					tau[p+j*nq] = tmp_tau[j];
					tau[p+(j+njoints)*nq] = tmp_dTau[j];
				}

	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("dotM(q)*dotq vector: \n");
				for (int k = 0; k < njoints; k++) 
					mexPrintf("%10.5f ", tmp_dTau[k]);
				mexPrintf("\n");
				
	#endif
				// Term 1 -> dotM(q)*dotq
				for (int i = 0; i < njoints; i++) {
					term_1[i] = tmp_dTau[i];
				}
				for (int k = 0; k < njoints; k++) {
					cumulative_res_term[k] = cumulative_res_term[k] + term_1[k];
					inner_res_term[p+k] = cumulative_res_term[k];
				}
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("Term 1: ");
				for (int k = 0; k < njoints; k++)
					mexPrintf("%10.5f ", term_1[k]);
				mexPrintf("\n");
	#endif
				
	#ifdef RESIDUAL_TERMS_DEBUG
				mexPrintf("\n********************************************************************\n\n");
	#endif	
			break;
			default:
				mexPrintf("\n*** Selected Residual calculus method does not exist ***\n");
				mexPrintf("- RES_CALCULUS_MODE determines how the residuals are going to be evaluated:\n");
				mexPrintf("0 -> just General-Purpose NE function calls will be used (NE);\n");
				mexPrintf("1 -> both General-Purpose NE and extended NE function calls will be used (Ext. NE);\n");
				mexPrintf("2 -> just Numerical NE function calls will be used (Cust. NE);\n");
				mexPrintf("3 -> both Numerical NE and extended RNE function calls will be used (Cust. Ext. NE);\n");
				mexPrintf("4 -> classical Euler-Lagrange method. M, S and g are computed numerically (Cust. EL);\n");
				mexPrintf("5 -> dotM*dotq in addiction to Numerical NE function calls (Paper [23] of the article).\n");
				mexPrintf("- IS_PLANAR_ROBOT determines which robot is passed as variable to 'residuals_NE(...)' function:\n");
				mexPrintf("0 -> KUKA LWR IV robot;\n");
				mexPrintf("1 -> N-LINK PLANAR robot.\n");
			break;
		}

		// End measuring the excution time
	#ifdef LINUX_MACHINE

		clock_gettime(CLOCK_MONOTONIC, &end);
		double elapsed_time_nsec = ts_to_ns(&end) - ts_to_ns(&start);
		elapsed_time_usec = elapsed_time_nsec / 1000.0;

	#else

		QueryPerformanceCounter(&end);
		elapsed_time_usec = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000000.0;

	#endif
		// mexPrintf("Execution time: %5.6f usec\n", elapsed_time_usec);

		comp_timing_term[p] = elapsed_time_usec;

		/****************************************/
		/******** END RESIDUALS CALCULUS ********/
		/****************************************/
        
	}

	mxFree(robot.links);
	mxFree(tau);
	mxFree(null_vec_nJoints);
	mxFree(cumulative_res_term);

	mxFree(term_1);
	for (int k = 0; k < njoints; k++) {
		mxFree(C_out[k]);
		mxFree(M[k]);
		mxFree(S[k]);
	}
	mxFree(C_out); mxFree(M); mxFree(S);

	mxFree(M_vec); mxFree(S_vec); mxFree(g_vec);
	// Numerical routine valid only when it is considering KUKA LWR IV
	mxFree(tmp_tau); mxFree(tmp_dTau);
	mxFree(tmp_q); mxFree(tmp_qd); mxFree(tmp_qdd); mxFree(tmp_qddd);
	mxFree(S_robot); mxFree(C_robot);

}

 /*
 *	Written by;
 *
 *		Giammarco Tonti & Corrado Guarino Lo Bianco
 *		University of Parma
 *		Via delle Scienze 181/A, 43124, Parma, PR, ITALY
 *
 *		giammarco.tonti@unipr.it
 *
 * Permission to use and distribute is granted, provided that this message
 * is retained, and due credit given when the results are incorporated in
 * publised work.
 *
 */

/**
 * Return the link rotation matrix and translation vector.
 *
 * @param l Link object for which R and p* are required.
 * @param th Joint angle, overrides value in link object
 * @param d Link extension, overrides value in link object
 * @param type Kinematic convention.
 */
static void
rot_mat (
	Link	*l,
	double	th,
	double	d,
	DHType	type
) {
	double		st, ct, sa, ca;

#ifdef	sun
	sincos(th, &st, &ct);
	sincos(l->alpha, &sa, &ca);
#else
	st = sin(th);
	ct = cos(th);
	sa = sin(l->alpha);
	ca = cos(l->alpha);
#endif

	switch (type) {
case STANDARD:
	l->R.n.x = ct;		l->R.o.x = -ca*st;	l->R.a.x = sa*st;
	l->R.n.y = st;		l->R.o.y = ca*ct;	l->R.a.y = -sa*ct;
	l->R.n.z = 0.0;		l->R.o.z = sa;		l->R.a.z = ca;

	l->r.x = l->A;
	l->r.y = d * sa;
	l->r.z = d * ca;
	break;
case MODIFIED:
	l->R.n.x = ct;		l->R.o.x = -st;		l->R.a.x = 0.0;
	l->R.n.y = st*ca;	l->R.o.y = ca*ct;	l->R.a.y = -sa;
	l->R.n.z = st*sa;	l->R.o.z = ct*sa;	l->R.a.z = ca;

	l->r.x = l->A;
	l->r.y = -d * sa;
	l->r.z = d * ca;
	break;
	}
}

/*************************************************************************
 * Matlab structure access methods
 *************************************************************************/

/*
static mxArray *
mstruct_get_element(mxArray *m, int i, char *field)
{
	mxArray	*e;

	if (mxIsCell(m)) {
#ifdef	DEBUG
		mexPrintf("%d x %d\n", mxGetM(m), mxGetN(m));
#endif
		get the i'th cell from the cell array */ /*
		if ((e = mxGetCell(m, i)) == NULL)
			error("get_element: field %s: cant get cell element %d",field, i);
	} else
		e = m;

	if (!mxIsStruct(e))
		mexErrMsgTxt("get_element: expecting a structure");
	if ((e = mxGetField(e, 0, field)) != NULL)
		return e;
	else {
		error("No such field as %s", field);
	}
} */

static mxArray *
mstruct_get_element(mxArray *m, int j, char *field)
{
    mxArray *e;

    if ((e = mxGetProperty(m, (mwIndex)j, field)) != NULL)
        return e;
    else {
        error("No such field as %s", field);
    }
}

static int
mstruct_getfield_number(mxArray *m, char *field)
{
	int	f;
	
	if ((f = mxGetFieldNumber(m, field)) < 0)
		error("no element %s in link structure");

	return f;
}

static int
mstruct_getint(mxArray *m, int i, char *field)
{
	mxArray	*e;

	e = mstruct_get_element(m, i, field);

	return (int) mxGetScalar(e);
}

static double
mstruct_getreal(mxArray *m, int i, char *field)
{
	mxArray	*e;

	e = mstruct_get_element(m, i, field);

	return mxGetScalar(e);
}

static double *
mstruct_getrealvect(mxArray *m, int i, char *field)
{
	mxArray	*e;

	e = mstruct_get_element(m, i, field);

	return mxGetPr(e);
}

#include	<stdarg.h>

/**
 * Error message handler.  Takes printf() style format string and variable
 * arguments and sends resultant string to Matlab via \t mexErrMsgTxt().
 *
 * @param s Error message string, \t  printf() style.
 */
void
error(char *s, ...)
{
	char	b[BUFSIZ];

	va_list	ap;

	va_start(ap, s);

	vsprintf(b, s, ap);

	mexErrMsgTxt(b);
}


/**
 * CORIOLIS_PAPER Compute the manipulator Coriolis matrix using RNE function calls (General-Purpose)
 * 
 * 	     CORIOLIS_PAPER(ROBOT, Q, QD, NUM_JOINTS, C)
 * 
 * Returns the matrix C(q,qd) corresponding to the Coriolis/centripetal forces. 
 * Coriolis/centripetal forces can be evaluated according to the following expression: tau = C(q,qd)*qd.
 * ROBOT is a robot object and describes the manipulator dynamics and kinematics, Q and QD are column vectors which contain the manipulator 
 * joint variables and their first derivatives, C is an n x n matrix, where n is the number of manipulator's joints.
 * 
 * Reference : 
 * H. Kawasaki, T. Bito, K. Kanzaki, "An efficient algorithm for the model-based adaptive control of robotic manipulators,” 
 * IEEE Trans. on Robotics, vol. 12, no. 3, pp. 496–501, 1996.
 * 
 */
void coriolis_Paper(Robot *robot, double *q, double *qd, int njoints, double **C_out)
{
	int n = robot->njoints;

	/****************************************/
	/*** Memory allocation of the vectors ***/
	/****************************************/

	// Collecting all q and qd for a single frne call
    int num_of_configurations = 2*n + 1;
    double q_[num_of_configurations*n];
    double qd_[num_of_configurations*n];
	double ei[n];
	double null_vec_nJoints_coriolis[n];
	for (int i = 0; i < njoints; i++)
		null_vec_nJoints_coriolis[i] = 0;
	double tau_coriolis[n];

	int column_counter = 0;

	double first_term[n], second_term[n], third_term[n];

	/****************************************/

	// Matrixes initialization
	for (int i = 0; i < n; i++) {
		q_[0*n+i] = q[i];
		qd_[0*n+i] = qd[i];
		for (int j = 0; j < n; j++)
			ei[j] = 0;
		ei[i] = 1;
		for (int j = 0; j < n; j++) {
			q_[((i)*2+1)*n+j] = q[j];
			q_[((i)*2+2)*n+j] = q[j];

			qd_[((i)*2+1)*n+j] = qd[j]+ei[j];
			qd_[((i)*2+2)*n+j] = ei[j];
		}
	}

	/*mexPrintf("\nqd_ matrix values:\n");
	for (int i=0; i<num_of_configurations; i++) {
		for (int j = 0; j < n; j++) 
			mexPrintf("%10.5f ", qd_[i*njoints + j]);
		mexPrintf("\n");
	}
	mexPrintf("\n");*/
    
    // Compute all points with a single FRNE function call
	robot->gravity->x = 0; robot->gravity->y = 0; robot->gravity->z = 0.0;
    for (int i = 0; i < num_of_configurations; i++) {

		newton_euler(robot, tau_coriolis, &qd_[i*n], null_vec_nJoints_coriolis, null_vec_nJoints_coriolis, 1);

		// Compute i-th column of the Coriolis matrix S/C
		if (i == 0) {
			for (int j = 0; j < n; j++) 
				second_term[j] = tau_coriolis[j];
		}
		if (i > 0 && i%2 == 1) {
			for (int j = 0; j < n; j++) 
				first_term[j] = tau_coriolis[j];
		}
		if (i > 0 && i%2 == 0) {
			for (int j = 0; j < n; j++) 
				third_term[j] = tau_coriolis[j];
			for (int j = 0; j < n; j++) 
				C_out[j][column_counter] = 1.0/2.0 * (first_term[j]-second_term[j]-third_term[j]);
			column_counter++;
		}
		
	}

	/*mexPrintf("\nC_out matrix values:\n");
	for (int i=0; i<n; i++) {
		for (int j = 0; j < n; j++) 
			mexPrintf("%10.4f ", C_out[i][j]);
		mexPrintf("\n");
	}
	mexPrintf("\n");*/

}

/**
 * CORIOLIS_PAPER_NUM Compute the manipulator Coriolis matrix using RNE function calls (Numerical) only for Kuka LWR IV and N-LINK PLANAR robots
 * 
 * 	     CORIOLIS_PAPER_NUM(Q, QD, NUM_JOINTS, C, S_IN, C_IN, IS_PLANAR_NUM)
 * 
 * Returns the matrix C(q,qd) corresponding to the Coriolis/centripetal forces. 
 * Coriolis/centripetal forces can be evaluated according to the following expression: tau = C(q,qd)*qd.
 * ROBOT is a robot object and describes the manipulator dynamics and kinematics, Q and QD are column vectors which contain the manipulator 
 * joint variables and their first derivatives, C is an n x n matrix, where n is the number of manipulator's joints.
 * 
 * Reference : 
 * H. Kawasaki, T. Bito, K. Kanzaki, "An efficient algorithm for the model-based adaptive control of robotic manipulators,” 
 * IEEE Trans. on Robotics, vol. 12, no. 3, pp. 496–501, 1996.
 * 
 */
void coriolis_Paper_num(double *q, double *qd, int njoints, double **C_out, double *S_in, double *C_in, int is_planar_robot)
{
	int n = njoints;
	int j = 0;

	/****************************************/
	/*** Memory allocation of the vectors ***/
	/****************************************/

	// Collecting all q and qd for a single frne call
    int num_of_configurations = 2*n + 1;
    double q_[num_of_configurations*n];
    double qd_[num_of_configurations*n];
	double ei[n];
	double null_vec_nJoints_coriolis[n];
	for (int i = 0; i < njoints; i++)
		null_vec_nJoints_coriolis[i] = 0;
	double tau_coriolis[n];

	int column_counter = 0;

	double first_term[n], second_term[n], third_term[n];

	double g_acc[3] = {0.0, 0.0, 0.0};

	/****************************************/

	// Matrixes initialization
	for (int i = 0; i < n; i++) {
		q_[0*n+i] = q[i];
		qd_[0*n+i] = qd[i];
		for (int j = 0; j < n; j++)
			ei[j] = 0;
		ei[i] = 1;
		for (int j = 0; j < n; j++) {
			q_[((i)*2+1)*n+j] = q[j];
			q_[((i)*2+2)*n+j] = q[j];

			qd_[((i)*2+1)*n+j] = qd[j]+ei[j];
			qd_[((i)*2+2)*n+j] = ei[j];
		}
	}

	/*mexPrintf("\nqd_ matrix values:\n");
	for (int i=0; i<num_of_configurations; i++) {
		for (int j = 0; j < n; j++) 
			mexPrintf("%10.5f ", qd_[i*njoints + j]);
		mexPrintf("\n");
	}
	mexPrintf("\n");*/

	// Compute all points with a single FRNE function call
	//robot->gravity->x = 0; robot->gravity->y = 0; robot->gravity->z = 0.0;
	// Numerical routine valid only when it is considering KUKA LWR IV

	for (int i = 0; i < num_of_configurations; i++) {
		if (!is_planar_robot)
			newton_euler_num_DH_grav(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
		else {
			// N-LINK PLANAR ROBOT
			switch (njoints)
			{
			case 3:
				newton_euler_num_DH_planar3(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 4:
				newton_euler_num_DH_planar4(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 5:
				newton_euler_num_DH_planar5(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 6:
				newton_euler_num_DH_planar6(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 7:
				newton_euler_num_DH_planar7(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 8:
				newton_euler_num_DH_planar8(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 9:
				newton_euler_num_DH_planar9(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 10:
				newton_euler_num_DH_planar10(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 11:
				newton_euler_num_DH_planar11(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 12:
				newton_euler_num_DH_planar12(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			case 13:
				newton_euler_num_DH_planar13(tau_coriolis, q, &qd_[i*n], null_vec_nJoints_coriolis, g_acc, S_in, C_in);
				break;
			default:
				break;
			}
		}

		// Compute i-th column of the Coriolis matrix S/C
		if (i == 0) {
			for (int j = 0; j < n; j++) 
				second_term[j] = tau_coriolis[j];
		}
		if (i > 0 && i%2 == 1) {
			for (int j = 0; j < n; j++) 
				first_term[j] = tau_coriolis[j];
		}
		if (i > 0 && i%2 == 0) {
			for (int j = 0; j < n; j++) 
				third_term[j] = tau_coriolis[j];
			for (int j = 0; j < n; j++) 
				C_out[j][column_counter] = 1.0/2.0 * (first_term[j]-second_term[j]-third_term[j]);
			column_counter++;
		}
	}

	/*mexPrintf("\nC_out matrix values:\n");
	for (int i=0; i<n; i++) {
		for (int j = 0; j < n; j++) 
			mexPrintf("%10.4f ", C_out[i][j]);
		mexPrintf("\n");
	}
	mexPrintf("\n");*/

}