
/**
 * \file ne_ext.c
 * \author Oscar Gerelli
 * \author Corrado Guarino Lo Bianco
 * \brief Compute the extended recursive Newton-Euler algorithm
 */

/*
 * Compute the extended inverse dynamics via the recursive Newton-Euler approach
 * according to the approaches proposed in:
 *
 * C. Guarino Lo Bianco (2009). Evaluation of generalized force derivatives by means 
 * of a recursive Newton-Euler approach. IEEE Trans. on Robotics, Vol. 25, NO. 4; p. 954-959
 *
 * and in
 *
 * C. Guarino Lo Bianco, E. Fantini (2006). A Recursive Newton-Euler Approach for the 
 * Evaluation of Generalized Forces Derivatives. In: 12th Int. Conf. on Methods and 
 * Models in Automation and Robotics. Międzyzdroje, Poland, August 28 - 31, p. 739-744
 *
 *	Requires:	qd	 current joint velocities
 *          	qdd	 current joint accelerations
 *          	qddd current joint jerks
 *              f	 applied tip force or load
 *              grav	the gravitational constant
 *
 *	Returns:	tau	vector of bias torques
 *              taud vector of the generalized force derivative
 */
#include	"residuals_NE.h"
#include	"vmath_ext.h"
#include	"ne_ext.h"

#include 	<stdlib.h>
#include 	<stdio.h>

/*
#define	DEBUG
*/

/*
 * Bunch of macros to make the main code easier to read.  Dereference vectors
 * from the Link structures for the manipulator.
 *
 * Note that they return pointers (except for M(j) which is a scalar)
 */
#undef	N

#define	OMEGA(j)	 (&links[j].omega)      /* angular velocity */
#define	OMEGADOT(j)	 (&links[j].omega_d)	/* angular acceleration */
#define OMEGADDOT(j) (&links[j].omega_dd)   /* angular jerk */
#define	ACC(j)		 (&links[j].acc)		/* linear acceleration */
#define	ACC_COG(j)	 (&links[j].abar)       /* linear acceln of COG */
#define JERK(j)      (&links[j].jerk)       /* linear jerk */
#define JERK_COG(j)  (&links[j].jcog)       /* linear jerk of COG */

#define	f(j)		 (&links[j].f)      /* force on link j due to link j-1 */
#define fd(j)        (&links[j].fd)     /* force derivative on link j due to link j-1 */
#define	n(j)		 (&links[j].n)      /* torque on link j due to link j-1 */
#define nd(j)        (&links[j].nd)     /* torque derivative on link j due to link j-1 */

#define	ROT(j)		 (&links[j].R)      /* link rotation matrix */
#define	M(j)		 (links[j].m)       /* link mass */
#define	PSTAR(j)	 (&links[j].r)      /* offset link i from link (j-1) */
#define	R_COG(j)	 (links[j].rbar)	/* COG link j wrt link j */
#define	INERTIA(j)	 (links[j].I)       /* inertia of link about COG */


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
) {
	Vect			t1, t2, t3, t4, t5;
	Vect			qdv, qddv, qdddv;
	Vect			F, N, Ndot, Fdot;
	Vect            z0 = {0.0, 0.0, 1.0};
	Vect            z1 = {1.0, 0.0, 0.0};
	Vect            z2 = {0.0, 1.0, 0.0};
	Vect            z3 = {0.0, 0.0, 1.0};
	Vect            zero = {0.0, 0.0, 0.0};
	Vect            f_tip = {0.0, 0.0, 0.0};
    Vect            fd_tip = {0.0, 0.0, 0.0};
	Vect            n_tip = {0.0, 0.0, 0.0};
    Vect            nd_tip = {0.0, 0.0, 0.0};
	register int	j;
/*	double			t; */
	Link			*links = robot->links;
    
	/*
	 * angular rate and acceleration vectors only have finite
	 * z-axis component
	 */
	qdv = qddv = qdddv = zero;

	/* setup external force/moment vectors */
	if (fext) {
		f_tip.x = fext[0];
		f_tip.y = fext[1];
		f_tip.z = fext[2];
		n_tip.x = fext[3];
		n_tip.y = fext[4];
		n_tip.z = fext[5];
	}

/******************************************************************************
 * forward recursion --the kinematics
 ******************************************************************************/

	if (robot->dhtype == MODIFIED) {
	    /*
	     * MODIFIED D&H CONVENTIONS
	     */
	    for (j = 0; j < robot->njoints; j++) {

		/* create angular vector from scalar input */
		qdv.z = qd[j*stride]; 
		qddv.z = qdd[j*stride];
		qdddv.z = qddd[j*stride];

		switch (links[j].sigma) {
		case REVOLUTE:

			/* 
			 * calculate angular velocity of link j
             *              
             * Attention: some temporary components of OMEGA(j) are
             * reused by OMEGADOT(j) and OMEGADDOT(j), so pay attention 
             * to any change introduced in OMEGA(j)
			 */
			if (j == 0)
				*OMEGA(j) = qdv;
			else {
				rot_trans_vect_mult (&t1, ROT(j), OMEGA(j-1));
				vect_add (OMEGA(j), &t1, &qdv);
			}

			/*
			 * calculate angular acceleration of link j 
             *              
             * Attention: some temporary components of OMEGADOT(j) are
             * reused by OMEGADDOT(j), so pay attention 
             * to any change introduced in OMEGADOT(j)
			 */
			if (j == 0) 
				*OMEGADOT(j) = qddv;
			else {
				rot_trans_vect_mult (&t2, ROT(j), OMEGADOT(j-1));
				vect_cross (&t3, &t1, &qdv);
				vect_add (&t3, &t3, &qddv);
				vect_add (OMEGADOT(j), &t2, &t3);
			}

			/*
			 * calculate angular jerk of link j 
			 */
			if (j == 0) 
				*OMEGADDOT(j) = qdddv;
			else {
                vect_add (&t3, &t3, &qddv);
                vect_cross (OMEGADDOT(j), &t1, &t3);
				rot_trans_vect_mult (&t3, ROT(j), OMEGADDOT(j-1));
				vect_add (OMEGADDOT(j), OMEGADDOT(j), &t3);
				vect_cross (&t3, &t2, &qdv);
				vect_add (OMEGADDOT(j), OMEGADDOT(j), &t3);
				vect_add (OMEGADDOT(j), OMEGADDOT(j), &qdddv);
			}

            /*
			 * compute acc[j]
             *
             * Attention: some temporary components of acc(j) are
             * reused by jerk(j), so pay attention 
             * to any change introduced in acc(j)
			 */
			if (j == 0) {
				t4 = *robot->gravity;
			} else {
				vect_cross(&t1, OMEGA(j-1), PSTAR(j));
				vect_cross(&t2, OMEGA(j-1), &t1);
				vect_cross(&t3, OMEGADOT(j-1), PSTAR(j));
				vect_add(&t3, &t3, &t2);
				vect_add(&t4, &t3, ACC(j-1));
			}
			rot_trans_vect_mult(ACC(j), ROT(j), &t4);

            /*
			 * compute jerk[j]
			 */
			if (j == 0) {
				*(JERK(j)) = zero;
			} else {
				vect_cross(&t4, OMEGA(j-1), &t3);
                scal_mult(&t1, &t1, 2.0);
				vect_cross(&t2, OMEGADOT(j-1), &t1);
				vect_add(&t4, &t4, &t2);
				vect_cross(&t2, OMEGADDOT(j-1), PSTAR(j));
                vect_add(&t4, &t4, &t2);
				vect_add(&t4, &t4, JERK(j-1));
                rot_trans_vect_mult(JERK(j), ROT(j), &t4);
			}

            break;

		case PRISMATIC:
            
			/* 
			 * calculate omega[j]
			 */
			if (j == 0)
				*(OMEGA(j)) = qdv;
			else
				rot_trans_vect_mult (OMEGA(j), ROT(j), OMEGA(j-1));

			/*
			 * calculate gamma[j] 
			 */
			if (j == 0)
				*(OMEGADOT(j)) = qddv;
			else
				rot_trans_vect_mult (OMEGADOT(j), ROT(j), OMEGADOT(j-1));


            /*
			 * calculate iota[j] 
			 */
			if (j == 0)
				*(OMEGADDOT(j)) = qdddv;
			else
				rot_trans_vect_mult (OMEGADDOT(j), ROT(j), OMEGADDOT(j-1));

            /*
			 * compute acc[j]
             *
             * Attention: some temporary components of acc(j) are
             * reused by jerk(j), so pay attention 
             * to any change introduced in acc(j)
			 */

            if (j == 0) {
				t4 = *robot->gravity;
                rot_trans_vect_mult(ACC(j), ROT(j), &t4);
			} else {
				vect_cross(&t1, OMEGA(j-1), PSTAR(j));
				vect_cross(&t2, OMEGA(j-1), &t1);
				vect_cross(&t3, OMEGADOT(j-1), PSTAR(j));
				vect_add(&t3, &t3, &t2);
				vect_add(&t4, &t3, ACC(j-1));
                rot_trans_vect_mult(ACC(j), ROT(j), &t4);
                
				vect_cross(&t4, OMEGA(j), &qdv);
                scal_mult(&t5, &t4, 2.0);
				vect_add(ACC(j), ACC(j), &t5);
			}
			vect_add(ACC(j), ACC(j), &qddv);

            /*
			 * compute jerk[j]
			 */
			if (j == 0) {
				*(JERK(j)) = zero;
			} else {
				vect_cross(&t5, OMEGA(j-1), &t3);
                scal_mult(&t1, &t1, 2.0);
				vect_cross(&t2, OMEGADOT(j-1), &t1);
				vect_add(&t5, &t5, &t2);
				vect_cross(&t2, OMEGADDOT(j-1), PSTAR(j));
                vect_add(&t5, &t5, &t2);
				vect_add(&t5, &t5, JERK(j-1));
                rot_trans_vect_mult(JERK(j), ROT(j), &t5);
                
                vect_add(&t5, &t4, &qddv);
 				vect_cross(&t4, OMEGA(j), &t5);
 				vect_cross(&t5, OMEGADOT(j), &qdv);
                vect_add(&t4, &t4, &t5);
                scal_mult(&t4, &t4, 3.0);
                vect_add(JERK(j), JERK(j), &t4);
			}
            vect_add(JERK(j), JERK(j), &qdddv);

			break;
		}

		/*
		 * compute abar[j]
		 */
		vect_cross(&t1, OMEGADOT(j), R_COG(j));
		vect_cross(&t2, OMEGA(j), R_COG(j));
		vect_cross(&t3, OMEGA(j), &t2);
		vect_add(&t4, &t1, &t3);
		vect_add(ACC_COG(j), &t4, ACC(j));
        
		/*
		 * compute jbar[j]
		 */
        
		vect_cross(JERK_COG(j), OMEGA(j), &t4);
		vect_cross(&t4, OMEGADOT(j), &t2);
        scal_mult(&t4, &t4, 2.0);
		vect_add(JERK_COG(j), JERK_COG(j), &t4);
		vect_cross(&t4, OMEGADDOT(j), R_COG(j));
		vect_add(JERK_COG(j), JERK_COG(j), &t4);
 		vect_add(JERK_COG(j), JERK_COG(j), JERK(j));

#ifdef	DEBUG
        mexPrintf("----------------------------------\n");
        mexPrintf(" Versione modificata: LINK %d\n", j);
		
		vect_print("w", OMEGA(j));
		vect_print("wd", OMEGADOT(j));
        vect_print("wdd", OMEGADDOT(j));
		vect_print("acc", ACC(j));
		vect_print("abar", ACC_COG(j));
        vect_print("jerk", JERK(j));
        vect_print("jcog", JERK_COG(j));
#endif

	    }
	} else {
	    /*
	     * STANDARD D&H CONVENTIONS
	     */
	    for (j = 0; j < robot->njoints; j++) {

		/* create angular vector from scalar input */
		qdv.z = qd[j*stride]; 
		qddv.z = qdd[j*stride];
        qdddv.z = qddd[j*stride];

		switch (links[j].sigma) {
		case REVOLUTE:
			/* 
			 * calculate omega[j]
			 */
			if (j == 0)
				t1 = qdv;
			else
				vect_add (&t1, OMEGA(j-1), &qdv);
			
            rot_trans_vect_mult (OMEGA(j), ROT(j), &t1);

			/*
			 * calculate alpha[j] 
			 */
			if (j == 0) 
				t3 = qddv;
			else {
				vect_add (&t1, OMEGADOT(j-1), &qddv);
				vect_cross (&t2, OMEGA(j-1), &qdv);
				vect_add (&t3, &t1, &t2);
			}
			rot_trans_vect_mult (OMEGADOT(j), ROT(j), &t3);

			/*
			 * calculate angular jerk[j] 
			 */
			if (j == 0) 
				t3 = qdddv;
			else {
				vect_add (&t1, OMEGADDOT(j-1), &qdddv);
				vect_cross (&t2, OMEGADOT(j-1), &qdv);
				vect_add (&t1, &t1, &t2);
                vect_cross (&t2, OMEGA(j-1), &qdv);
                scal_mult(&t4, &qddv, 2.0);
                vect_add (&t3, &t2, &t4);
                vect_cross (&t2, OMEGA(j-1), &t3);
                vect_add (&t3, &t1, &t2);
			}
			rot_trans_vect_mult (OMEGADDOT(j), ROT(j), &t3);
            
			/*
			 * compute acc[j]
             *
             * Attention: some temporary components of acc(j) are
             * reused by jerk(j), so pay attention to any change 
             * introduced in acc(j)
			 */
            
			vect_cross(&t1, OMEGADOT(j), PSTAR(j));
			vect_cross(&t2, OMEGA(j), PSTAR(j));
			vect_cross(&t3, OMEGA(j), &t2);
			vect_add(&t4, &t1, &t3);
			if (j == 0) {
				rot_trans_vect_mult(&t5, ROT(j), robot->gravity);
			} else 
				rot_trans_vect_mult(&t5, ROT(j), ACC(j-1));
            
			vect_add(ACC(j), &t5, &t4);

   			/*
			 * compute linear jerk[j]
             * Attention: some temporary components of jerk[j] are
             * obtained from acc(j), so pay attention to any change 
             * introduced in acc(j)
			 */
            
            vect_cross(JERK(j), OMEGA(j), &t4);
            
            scal_mult(&t4, &t2, 2.0);
            vect_cross(&t5, OMEGADOT(j), &t4);
            vect_add(JERK(j), JERK(j), &t5);
            
			vect_cross(&t5, OMEGADDOT(j), PSTAR(j));
            vect_add(JERK(j), JERK(j), &t5);
            
            if (j!=0) {
                rot_trans_vect_mult(&t1, ROT(j), JERK(j-1));
                vect_add(JERK(j), JERK(j), &t1);
            }
            
			break;

		case PRISMATIC:
			/* 
			 * calculate omega[j]
			 */
			if (j == 0)
				*(OMEGA(j)) = zero;
			else
				rot_trans_vect_mult (OMEGA(j), ROT(j), OMEGA(j-1));

			/*
			 * calculate alpha[j] 
			 */
			if (j == 0)
				*(OMEGADOT(j)) = zero;
			else
				rot_trans_vect_mult (OMEGADOT(j), ROT(j), OMEGADOT(j-1));

			/*
			 * calculate jerk[j] 
			 */
			if (j == 0) 
				*(OMEGADDOT(j)) = zero;
			else
                rot_trans_vect_mult (OMEGADDOT(j), ROT(j), OMEGADDOT(j-1));

            
			/*
			 * compute acc[j]
             *
             * Attention: some temporary components of acc(j) are
             * reused by jerk(j), so pay attention to any change 
             * introduced in acc(j)
			 */
            
			vect_cross(&t1, OMEGADOT(j), PSTAR(j));
			vect_cross(&t2, OMEGA(j), PSTAR(j));
			vect_cross(&t3, OMEGA(j), &t2);
			vect_add(&t4, &t1, &t3);
			if (j == 0) {
				rot_trans_vect_mult(&t5, ROT(j), robot->gravity);
			} else 
				rot_trans_vect_mult(&t5, ROT(j), ACC(j-1));
            
			vect_add(ACC(j), &t5, &t4);

            rot_trans_vect_mult(&t1, ROT(j), &qdv);
            scal_mult(&t3, &t1, 2.0);
            vect_cross(&t5, OMEGA(j), &t3);
			vect_add(ACC(j), ACC(j), &t5);
            
            rot_trans_vect_mult(&t3, ROT(j), &qddv);
			vect_add(ACC(j), ACC(j), &t3);
            
            
   			/*
			 * compute linear jerk[j]
             * Attention: some temporary components of jerk(j) are
             * obtained from acc(j), so pay attention to any change 
             * introduced in acc(j)
			 */
            
            scal_mult(&t1, &t1, 3.0); /* 3d_punto */
            vect_cross(&t5, OMEGA(j), &t1);
            vect_add(&t4, &t4, &t5);
            
            scal_mult(&t3, &t3, 3.0); /* 3dd_punto */
            vect_add(&t4, &t4, &t3);
            vect_cross(JERK(j), OMEGA(j), &t4);
            
            scal_mult(&t2, &t2, 2.0);
            vect_add(&t2, &t2, &t1);
            vect_cross(&t4, OMEGADOT(j), &t2);
            vect_add(JERK(j), JERK(j), &t4);
            
            rot_trans_vect_mult(&t4, ROT(j), &qdddv);
            vect_add(JERK(j), JERK(j), &t4);

            vect_cross(&t4, OMEGADDOT(j), PSTAR(j));
            vect_add(JERK(j), JERK(j), &t4);

            if (j != 0){
				rot_trans_vect_mult(&t4, ROT(j), JERK(j-1));
                vect_add(JERK(j), JERK(j), &t4);
            }

			break;
		}
		/*
		 * compute abar[j]
         *
         * Attention: some temporary components of acc_cog(j) are
         * reused by jerk_cog(j), so pay attention to any change 
         * introduced in acc_corg(j)
		 */
        
		vect_cross(&t1, OMEGADOT(j), R_COG(j));
		vect_cross(&t2, OMEGA(j), R_COG(j));
		vect_cross(&t3, OMEGA(j), &t2);
		vect_add(&t4, &t1, &t3);
		vect_add(ACC_COG(j), &t4, ACC(j));

        /*
         * compute linear jerk of the COG jerk_cog[j]
         * Attention: some temporary components of jerk_cog[j] are
         * obtained from acc_cog(j), so pay attention to any change 
         * introduced in acc_cog(j)
         */
        
        vect_cross(JERK_COG(j), OMEGA(j), &t4);
        
        scal_mult(&t2, &t2, 2.0);
        vect_cross(&t4, OMEGADOT(j), &t2);
        vect_add(JERK_COG(j), JERK_COG(j), &t4);
        
        vect_cross(&t4, OMEGADDOT(j), R_COG(j));
        vect_add(JERK_COG(j), JERK_COG(j), &t4);
        vect_add(JERK_COG(j), JERK_COG(j), JERK(j));

        
#ifdef	DEBUG
        mexPrintf("----------------------------------\n");
        mexPrintf(" Versione modificata: LINK %d\n", j);
		vect_print("w", OMEGA(j));
		vect_print("wd", OMEGADOT(j));
        vect_print("wdd", OMEGADDOT(j));
		vect_print("acc", ACC(j));
		vect_print("abar", ACC_COG(j));
        vect_print("jerk", JERK(j));
        vect_print("jcog", JERK_COG(j));
#endif
	    }
	}

/******************************************************************************
 * backward recursion part --the kinetics
 ******************************************************************************/

	if (robot->dhtype == MODIFIED) {
	    /*
	     * MODIFIED D&H CONVENTIONS
	     */
	    for (j = robot->njoints - 1; j >= 0; j--) {

		/*
		 * compute F[j]
		 */
		scal_mult (&F, ACC_COG(j), M(j));

		/*
		 * compute Fdot[j]
		 */
		scal_mult (&Fdot, JERK_COG(j), M(j));

        /*
		 * compute f[j]
         * Attention: intermediate variable t1 is
         * reused in the following for evaluating ndot:
         * it cannot be reassigned in the meanwhile
		 */
        
		if (j == (robot->njoints-1))
			t1 = f_tip;
		else
			rot_vect_mult (&t1, ROT(j+1), f(j+1));
        
		vect_add (f(j), &t1, &F);

        /*
		 * compute fdot[j]
         * Attention: intermediate variable t2 is
         * reused in the following for evaluating ndot[j]:
         * it cannot be reassigned in the meanwhile
		 */
        
		if (j == (robot->njoints-1))
			t2 = fd_tip;
		else
			rot_vect_mult (&t2, ROT(j+1), fd(j+1));
        
		vect_add (fd(j), &t2, &Fdot);
        

		 /*
		  * compute N[j]
          * Attention: intermediate variable t3 and t4
          * reused in the following for evaluating Ndot[j]:
          * it cannot be reassigned in the meanwhile
		  */

        mat_vect_mult(&t3, INERTIA(j), OMEGADOT(j));
		mat_vect_mult(&t4, INERTIA(j), OMEGA(j));
		vect_cross(&t5, OMEGA(j), &t4);
		vect_add(&N, &t3, &t5);
        
         /*
		  * compute Ndot[j]
		  */

        vect_cross(&Ndot, OMEGA(j), &t3);
        vect_cross(&t3, OMEGADOT(j), &t4);
        vect_add(&Ndot, &Ndot, &t3);
        vect_cross(&t3, OMEGA(j), &N);
        vect_add(&Ndot, &Ndot, &t3);
        
        vect_cross(&t3, OMEGADOT(j), OMEGA(j));
        vect_add(&t3, &t3, OMEGADDOT(j));
        
        mat_vect_mult(&t4, INERTIA(j), &t3);
        vect_add(&Ndot, &Ndot, &t4);
        
		 /*
		  * compute n[j]
          * The following equations are valid only 
          * if frame N+1 coincides with frame N
		  */
		if (j == (robot->njoints-1))
			t3 = n_tip;
		else {
			rot_vect_mult(&t3, ROT(j+1), n(j+1));
			rot_vect_mult(&t4, ROT(j+1), f(j+1));
			vect_cross(&t5, PSTAR(j+1), &t4);

			vect_add(&t3, &t3, &t5);
		}

		vect_cross(&t4, R_COG(j), &F);
		vect_add(&t3, &t3, &t4);
		vect_add(n(j), &t3, &N);
        
		 /*
		  * compute ndot[j]
          * The following equations are only valid  
          * if frame N+1 coincides with frame N
          *
          * Terms t1 and t2 have been evaluated in f[j] and fd[j]
		  */
		if (j == (robot->njoints-1))
			t3 = nd_tip;
		else {
			vect_cross(&t4, OMEGA(j), PSTAR(j+1));
            switch (links[j+1].sigma) {
                case PRISMATIC:
                    qdv.z = qd[(j+1)*stride];
                    rot_vect_mult(&t3, ROT(j+1), &qdv);
                    vect_add(&t4, &t3, &t4);
                }
            vect_cross(&t3, &t4, &t1);
                
			rot_vect_mult(&t4, ROT(j+1), nd(j+1));
            vect_add(&t3, &t3, &t4);
            
            vect_cross(&t4, PSTAR(j+1), &t2);
            vect_add(&t3, &t3, &t4);
    	}
		vect_cross(&t5, OMEGA(j), R_COG(j));
		vect_cross(&t4, &t5, &F);
        vect_add(nd(j), &t3, &t4);

		vect_cross(&t3, R_COG(j), &Fdot);
        vect_add(nd(j), nd(j), &t3);

        vect_add(nd(j), nd(j), &Ndot);
        
#ifdef	DEBUG
		vect_print("f", f(j));    
		vect_print("n", n(j));
#endif
	    }

	} else {
	    /*
	     * STANDARD D&H CONVENTIONS
	     */
	    for (j = robot->njoints - 1; j >= 0; j--) {

    	/*
		 * compute F[j]
		 */
		scal_mult (&F, ACC_COG(j), M(j));

		 /*
		  * compute N[j]
          *
          * Attention: some temporary components of N are
          * reused by Ndot, so pay attention to any change 
          * introduced in N
		  */

        mat_vect_mult(&t1, INERTIA(j), OMEGADOT(j));
		mat_vect_mult(&t2, INERTIA(j), OMEGA(j));
		vect_cross(&t3, OMEGA(j), &t2);
		vect_add(&N, &t1, &t3);

        /*
		 * compute Fdot[j]
		 */
		scal_mult (&Fdot, JERK_COG(j), M(j));

		 /*
		  * compute Ndot[j]
          * Attention: some temporary components of Ndot are
          * obtained from N, so pay attention to any change 
          * introduced in N
		  */
        
        vect_cross(&Ndot, OMEGA(j), &t1);
        
        vect_cross(&t1, OMEGADOT(j), &t2);
        vect_add(&Ndot, &Ndot, &t1);
        
        vect_cross(&t1, OMEGA(j), OMEGADOT(j));
        mat_vect_mult(&t2, INERTIA(j), &t1);
        scal_mult(&t2, &t2, -1.0);
        vect_add(&Ndot, &Ndot, &t2);
        
        vect_cross(&t1, OMEGA(j), &N);
        vect_add(&Ndot, &Ndot, &t1);
        
        mat_vect_mult(&t1, INERTIA(j), OMEGADDOT(j));
        vect_add(&Ndot, &Ndot, &t1);

        /*
		 * compute f[j]
		 */
		if (j != (robot->njoints-1)) {
			rot_vect_mult (&t1, ROT(j+1), f(j+1));
			vect_add (f(j), &F, &t1);
		} else
			vect_add (f(j), &F, &f_tip);
       
		 /*
		  * compute n[j]
		  */		        
        
		if (j != (robot->njoints-1)) {
			rot_vect_mult (&t1, ROT(j+1), n(j+1));
		} else
			t1 = n_tip;
        vect_cross(&t2,PSTAR(j),f(j));
        vect_add(n(j),&t1,&t2);
        vect_cross(&t1,R_COG(j),&F);
        vect_add(n(j),&t1,n(j));
        vect_add(n(j),&N,n(j));
      
        /*
		 * compute fdot[j]
		 */
		if (j != (robot->njoints-1)) {
			rot_vect_mult (&t1, ROT(j+1), fd(j+1));
			vect_add (fd(j), &Fdot, &t1);
		} else
			vect_add (fd(j), &Fdot, &fd_tip);

	   /*
	    * compute nd[j]
	    */		        		
        if (j == (robot->njoints-1))
			t1 = nd_tip;
		else {
			rot_vect_mult(&t1, ROT(j+1), nd(j+1));
		}
    	vect_cross(&t2, R_COG(j), &Fdot);
      	vect_add(nd(j), &t1, &t2);
        
        vect_cross(&t2, OMEGA(j), R_COG(j));
        vect_cross(&t1, &t2, &F);
        vect_add(nd(j), nd(j), &t1);

        vect_cross(&t1, PSTAR(j), fd(j));
        vect_add(nd(j), nd(j), &t1);
        
        vect_cross(&t3, OMEGA(j), PSTAR(j));
        switch (links[j].sigma) {
		case PRISMATIC:
          qdv.z = qd[j*stride];
    	  rot_trans_vect_mult(&t1, ROT(j), &qdv);
          vect_add(&t3, &t3, &t1);
        }
        vect_cross(&t1, &t3, f(j));
        vect_add(nd(j), nd(j), &t1);
        vect_add(nd(j), nd(j), &Ndot);       
        
#ifdef	DEBUG
		vect_print("F", &F);
		vect_print("Fd", &Fdot);
		vect_print("N", &N);
		vect_print("Nd", &Ndot);        
		vect_print("f", f(j));
        vect_print("fd", fd(j));
		vect_print("n", n(j));
        vect_print("nd", nd(j)); 
#endif
	    }
	}

	/*
	 *  Compute the total torque for each axis
	 *
	 */
	for (j=0; j < robot->njoints; j++) {
		double	t, tc, td;
/*        int k; */

		Link	*l = &links[j];

		if (robot->dhtype == MODIFIED)
			t1 = z0;
		else
			rot_trans_vect_mult(&t1, ROT(j), &z0);

		switch (l->sigma) {
		case REVOLUTE:
			t = vect_dot(n(j), &t1);
            
            vect_cross(&t3, n(j), OMEGA(j));
            vect_add(&t2, nd(j), &t3);
            td = vect_dot(&t2, &t1);
			break;
		case PRISMATIC:
			t = vect_dot(f(j), &t1);

            vect_cross(&t3, f(j), OMEGA(j));
            vect_add(&t2, fd(j), &t3);
            td = vect_dot(&t2, &t1);            
			break;
		}
        
        /*
		 * add actuator dynamics and friction
		 */
        
        tc = t; 
        
 		tc = t + l->G * l->G * (
 			l->Jm * qdd[j*stride] +
 			l->B * qd[j*stride] +
 			(qd[j*stride] > 0 ? l->Tc[0] : 0.0) +
 			(qd[j*stride] < 0 ? l->Tc[1] : 0.0)
 		);
        
 		td = td + l->G * l->G * (
 			l->Jm * qddd[j*stride] +
 			l->B * qdd[j*stride]
 		);
       
        /*
         * Do not delete the following lines:
         * they have been added for debug purposes
         */
        
        /* scal_mult(&t1, OMEGADOT(j), 1.0);
        scal_mult(&t2, OMEGADDOT(j), 1.0);
                
        for (k=j; k >= 0; k--) {
            rot_vect_mult(&t3, ROT(k), &t1);
            rot_vect_mult(&t4, ROT(k), &t2);
            t1 = t3;
            t2 = t4;
        } 
        td = vect_dot(&t1,&z1);
        t = vect_dot(&t2,&z1); */
        
        //t=vect_dot(&qdv,&z3); 
        
        tau[j*stride] = tc;
        tau[(robot->njoints + j)*stride] = td;
		/* tau[(2*(robot->njoints) + j)*stride] = t; */

        /*#ifdef	DEBUG
            vect_print("t", &t);
            vect_print("tc", &tc);
            vect_print("td", &td);
        #endif*/
	}
}
